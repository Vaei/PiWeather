#include <vector>
#include <memory>
#include <iostream>
#include <csignal>
#include <SDL2/SDL.h>

#include "settingsmanager.h"
#include "widgets/widget.h"
#include "widgets/counterwidget.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

// Constants
const std::string configDir = std::string(getenv("HOME")) + "/.config/PiWeather";
const std::string generalSettingsPath = configDir + "/settings.ini";
const std::string widgetSettingsPath = configDir + "/widgets.ini";
const std::string appVersion = "1.0.0";

volatile bool running = true;

void handleSignal(int signal) {
    if (signal == SIGINT) {
        running = false;
    }
}

void ensureDirectoryExists(const std::string& path) {
    if (system(("mkdir -p " + path).c_str()) != 0) {
        std::cerr << "Failed to create directory: " << path << std::endl;
    }
}

int main() {
    // Ensure configuration directory exists
    ensureDirectoryExists(configDir);

    std::string lastUsedPreset;
    std::vector<std::unique_ptr<Widget>> widgets;

    // Load general settings
    if (!SettingsManager::loadGeneralSettings(generalSettingsPath, lastUsedPreset, appVersion)) {
        // std::cerr << "Failed to load general settings. Creating new settings file.\n";
        SettingsManager::saveGeneralSettings(generalSettingsPath, "Default", appVersion);
        lastUsedPreset = "Default";
    }

    // Load widgets for the preset
    if (!SettingsManager::loadWidgets(generalSettingsPath, lastUsedPreset, widgets)) {
        // std::cerr << "No widgets found for preset '" << lastUsedPreset << "'. Using default widgets.\n";
        widgets.emplace_back(std::make_unique<CounterWidget>("DefaultCounter", 0.5f, 0.5f, CounterWidget::CounterMode::Seconds, 0));
    }
    
    // Initialize SDL and FFmpeg
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("PiWeather", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 600, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // FFmpeg setup for video playback
    AVFormatContext* formatContext = nullptr;
    const char* videoPath = "/home/pi/Videos/PiWeather/totoro_001.mp4";
    if (avformat_open_input(&formatContext, videoPath, nullptr, nullptr) != 0) {
        std::cerr << "Failed to open video file: " << videoPath << std::endl;
        return -1;
    }
    avformat_find_stream_info(formatContext, nullptr);

    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; ++i) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    const AVCodec* codec = avcodec_find_decoder(formatContext->streams[videoStreamIndex]->codecpar->codec_id);
    AVCodecContext* codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecContext, formatContext->streams[videoStreamIndex]->codecpar);
    avcodec_open2(codecContext, codec, nullptr);

    SwsContext* swsContext = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
                                            1024, 600, AV_PIX_FMT_YUV420P, SWS_BILINEAR, nullptr, nullptr, nullptr);

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, 1024, 600);
    AVFrame* frame = av_frame_alloc();
    AVFrame* frameYUV = av_frame_alloc();
    frameYUV->format = AV_PIX_FMT_YUV420P;
    frameYUV->width = 1024;
    frameYUV->height = 600;
    av_frame_get_buffer(frameYUV, 32);

    AVPacket packet;
    SDL_Event event;

    // Main loop
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = false;
                break;
            }
        }

        if (av_read_frame(formatContext, &packet) >= 0) {
            if (packet.stream_index == videoStreamIndex) {
                avcodec_send_packet(codecContext, &packet);
                if (avcodec_receive_frame(codecContext, frame) == 0) {
                    sws_scale(swsContext, frame->data, frame->linesize, 0, codecContext->height,
                              frameYUV->data, frameYUV->linesize);
                    SDL_UpdateYUVTexture(texture, nullptr, frameYUV->data[0], frameYUV->linesize[0],
                                         frameYUV->data[1], frameYUV->linesize[1],
                                         frameYUV->data[2], frameYUV->linesize[2]);

                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, texture, nullptr, nullptr);

                    // Render widgets over video
                    for (const auto& widget : widgets) {
                        widget->render(renderer);
                    }

                    SDL_RenderPresent(renderer);
                }
            }
            av_packet_unref(&packet);
        } else {
            av_seek_frame(formatContext, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
            avcodec_flush_buffers(codecContext);
        }
    }

    av_frame_free(&frame);
    av_frame_free(&frameYUV);
    sws_freeContext(swsContext);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
