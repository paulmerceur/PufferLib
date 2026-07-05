#include "four_rooms.h"
#include "puffernet.h"
#include <limits.h>
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

typedef struct {
    FourRooms* env;
    PufferNet* net;
} Demo;

static void get_input(FourRooms* env) {
    env->actions[0] = DONE;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) env->actions[0] = FORWARD;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) env->actions[0] = LEFT;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) env->actions[0] = RIGHT;
}

static void step_demo(Demo* demo) {
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        get_input(demo->env);
    } else {
        for (int i = 0; i < FOUR_ROOMS_VIEW_SIZE * FOUR_ROOMS_VIEW_SIZE * FOUR_ROOMS_OBS_CHANNELS; i++) {
            demo->net->obs[i] = (float)demo->env->observations[i];
        }
        forward_puffernet(demo->net, demo->net->obs, demo->env->actions);
    }

    c_step(demo->env);
    demo->env->episode_steps = INT_MAX;
    c_render(demo->env);
}

#ifdef __EMSCRIPTEN__
static void emscripten_step(void* data) {
    step_demo((Demo*)data);
}
#endif

int main() {
    srand((unsigned int)time(NULL));

    FourRooms* env = (FourRooms*)calloc(1, sizeof(FourRooms));
    env->size = 19;
    env->max_steps = 0;
    env->num_agents = 1;
    env->rng = (unsigned int)time(NULL);
    allocate(env);

    Weights* weights = load_weights("resources/four_rooms/four_rooms_weights.bin");
    int logit_sizes[1] = {FOUR_ROOMS_NUM_ACTIONS};
    PufferNet* net = make_puffernet(
        weights,
        1,
        FOUR_ROOMS_VIEW_SIZE * FOUR_ROOMS_VIEW_SIZE * FOUR_ROOMS_OBS_CHANNELS,
        128,
        2,
        logit_sizes,
        1
    );

    c_reset(env);
    env->episode_steps = INT_MAX;
    c_render(env);

    Demo demo = {.env = env, .net = net};

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(emscripten_step, &demo, 0, true);
#else
    while (!WindowShouldClose()) {
        step_demo(&demo);
    }

    free_puffernet(net);
    free(weights);
    free_allocated(env);
    free(env);
#endif
    return 0;
}
