package main

import "core:fmt"
import rl "vendor:raylib"

Ball :: struct {
    position: rl.Vector2,
    velocity: rl.Vector2,
    radius: f32,
    color: rl.Color
}

main :: proc() {
    rl.InitWindow(800, 450, "JPS - JoJo's Physics Simulator")

    balls: [dynamic]Ball

    append(&balls, Ball{rl.Vector2{400, 225}, rl.Vector2{0, 0}, 10, rl.MAROON})

    for !rl.WindowShouldClose() {
        rl.BeginDrawing()
            rl.ClearBackground(rl.RAYWHITE)
            
            for ball in balls {
                rl.DrawCircleV(ball.position, ball.radius, ball.color)
            }

        rl.EndDrawing()
    }

    rl.CloseWindow()
}