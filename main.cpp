#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <string>

// Lớp Texture quản lý việc tải và hiển thị hình ảnh
class Texture {
private:
    SDL_Texture* mTexture;
    SDL_Renderer* mRenderer;
    int mWidth;
    int mHeight;

public:
    // Khởi tạo biến
    Texture(SDL_Renderer* renderer) {
        mTexture = nullptr;
        mRenderer = renderer;
        mWidth = 0;
        mHeight = 0;
    }

    // Giải phóng bộ nhớ
    ~Texture() {
        free();
    }

    // Giải phóng texture
    void free() {
        if (mTexture != nullptr) {
            SDL_DestroyTexture(mTexture);
            mTexture = nullptr;
            mWidth = 0;
            mHeight = 0;
        }
    }

    // Tải hình ảnh từ đường dẫn
    bool loadFromFile(std::string path) {
        // Giải phóng texture cũ nếu có
        free();

        // Texture cuối cùng
        SDL_Texture* newTexture = nullptr;

        // Tải hình ảnh từ đường dẫn
        SDL_Surface* loadedSurface = IMG_Load(path.c_str());
        if (loadedSurface == nullptr) {
            std::cout << "Không thể tải hình ảnh " << path.c_str() << "! SDL_image Error: " << IMG_GetError() << std::endl;
        } else {
            // Tạo texture từ surface
            newTexture = SDL_CreateTextureFromSurface(mRenderer, loadedSurface);
            if (newTexture == nullptr) {
                std::cout << "Không thể tạo texture từ " << path.c_str() << "! SDL Error: " << SDL_GetError() << std::endl;
            } else {
                // Lấy kích thước hình ảnh
                mWidth = loadedSurface->w;
                mHeight = loadedSurface->h;
            }

            // Giải phóng surface đã tải
            SDL_FreeSurface(loadedSurface);
        }

        // Trả về success/fail
        mTexture = newTexture;
        return mTexture != nullptr;
    }

    // Vẽ texture lên màn hình
    void render(int x, int y) {
        // Thiết lập vùng render
        SDL_Rect renderQuad = { x, y, mWidth, mHeight };

        // Render texture vào màn hình
        SDL_RenderCopy(mRenderer, mTexture, nullptr, &renderQuad);
    }

    // Lấy kích thước
    int getWidth() { return mWidth; }
    int getHeight() { return mHeight; }
};

int main(int argc, char* argv[]) {
    // Khởi tạo SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Khởi tạo SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cout << "SDL_image không thể khởi tạo! SDL_image Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Tạo cửa sổ
    SDL_Window* window = SDL_CreateWindow("Hiển thị ảnh với SDL2", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        640, 480, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Tạo renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Tạo đối tượng texture
    Texture backgroundTexture(renderer);

    // Tải hình ảnh từ đường dẫn
    if (!backgroundTexture.loadFromFile("assets\\background.png")) {
        std::cout << "Không thể tải hình ảnh nền!" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Vòng lặp chính
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        // Xử lý sự kiện
        while (SDL_PollEvent(&e) != 0) {
            // Sự kiện thoát
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Xóa màn hình
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        // Vẽ hình ảnh nền
        backgroundTexture.render(0, 0);

        // Cập nhật màn hình
        SDL_RenderPresent(renderer);
    }

    // Giải phóng tài nguyên
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}