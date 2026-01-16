#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>

#define WIDTH 900
#define HEIGHT 600

constexpr float GRAVITY = 0.25f;

struct Circle {
    float x, y;
    float radius;
    float vx, vy;

    Circle(float x, float y, float r)
        : x(x), y(y), radius(r), vx(0.0f), vy(0.0f) {}
};

float random_range(float min, float max)
{
    return min + (rand() / (float)RAND_MAX) * (max - min);
}

int SDL_RenderFillCircle(SDL_Renderer* r, const Circle& c)
{
    int ox = 0;
    int oy = (int)c.radius;
    int d = (int)c.radius - 1;

    while (oy >= ox) {
        SDL_RenderDrawLine(r, c.x - oy, c.y + ox, c.x + oy, c.y + ox);
        SDL_RenderDrawLine(r, c.x - ox, c.y + oy, c.x + ox, c.y + oy);
        SDL_RenderDrawLine(r, c.x - ox, c.y - oy, c.x + ox, c.y - oy);
        SDL_RenderDrawLine(r, c.x - oy, c.y - ox, c.x + oy, c.y - ox);

        if (d >= 2 * ox) {
            d -= 2 * ox + 1;
            ox++;
        } else if (d < 2 * (c.radius - oy)) {
            d += 2 * oy - 1;
            oy--;
        } else {
            d += 2 * (oy - ox - 1);
            oy--;
            ox++;
        }
    }
    return 0;
}

void update_circle(Circle& c)
{
    c.vy += GRAVITY;
    c.x += c.vx;
    c.y += c.vy;

    c.vx *= 0.999f;
    c.vy *= 0.999f;

    if (c.y + c.radius > HEIGHT) {
        c.y = HEIGHT - c.radius;
        c.vy = -c.vy * 0.9f;
    }
    if (c.y - c.radius < 0) {
        c.y = c.radius;
        c.vy = -c.vy;
    }
    if (c.x - c.radius < 0) {
        c.x = c.radius;
        c.vx = -c.vx;
    }
    if (c.x + c.radius > WIDTH) {
        c.x = WIDTH - c.radius;
        c.vx = -c.vx;
    }
}

void resolve_collision(Circle& a, Circle& b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dist = std::sqrt(dx * dx + dy * dy);
    float minDist = a.radius + b.radius;

    if (dist < minDist && dist > 0.0f) {
        float nx = dx / dist - 0.5f;
        float ny = dy / dist - 0.5f;

        float overlap = 0.5f * (minDist - dist);
        a.x -= nx * overlap + 0.5f;
        a.y -= ny * overlap + 0.5f;
        b.x += nx * overlap + 0.5f;
        b.y += ny * overlap + 0.5f;

        float k = 1.1f;
        a.vx -= nx * k;
        a.vy -= ny * k;
        b.vx += nx * k;
        b.vy += ny * k;

        a.vy -= 1.1f;
        b.vy += 1.1f;
    }
}

int main()
{
    srand((unsigned)time(nullptr));

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Bouncy Circles", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    std::vector<Circle> circles;
    circles.reserve(100);
    circles.emplace_back(WIDTH / 2.0f, HEIGHT - 75.0f, 20.0f);

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_s && circles.size() < 100) {
                    circles.emplace_back(WIDTH / 2.0f, HEIGHT - 80.0f, 20.0f);
                    circles.back().vx = random_range(-5.0f, 5.0f);
                    circles.back().vy = random_range(-15.0f, -8.0f);
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (auto& c : circles)
            update_circle(c);

        for (size_t i = 0; i < circles.size(); ++i)
            for (size_t j = i + 1; j < circles.size(); ++j)
                resolve_collision(circles[i], circles[j]);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (auto& c : circles)
            SDL_RenderFillCircle(renderer, c);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
