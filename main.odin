package main

import "core:fmt"
import "core:strings"
import rl "vendor:raylib"

Ball :: struct {
	position: rl.Vector2,
	velocity: rl.Vector2,
	radius:   f32,
	color:    rl.Color,
}

update :: proc(ball: ^Ball, dt: f32) {
	g: f32 = 9.81 * 50
	d: f32 = 0.99 / 5

	ball.velocity = rl.Vector2Add(ball.velocity, rl.Vector2Scale(rl.Vector2{0, g}, dt))
	dragForce := rl.Vector2Scale(ball.velocity, -d);
    ball.velocity = rl.Vector2Add(ball.velocity, rl.Vector2Scale(dragForce, dt));


	if ((ball.position.x - ball.radius) <= 0 ||
		   (ball.position.x + ball.radius) >= f32(rl.GetScreenWidth())) {
		ball.velocity.x *= -1
	}
	if ((ball.position.y - ball.radius) <= 0 ||
		   (ball.position.y + ball.radius) >= f32(rl.GetScreenHeight())) {
		ball.velocity.y *= -1
	}

	ball.position = rl.Vector2Add(ball.position, rl.Vector2Scale(ball.velocity, dt))
}

main :: proc() {
	rl.InitWindow(800, 450, "JPS - JoJo's Physics Simulator")
	rl.SetTargetFPS(120)

	balls: [dynamic]Ball

	append(&balls, Ball{rl.Vector2{400, 225}, rl.Vector2{0, 0}, 10, rl.MAROON})

	for !rl.WindowShouldClose() {
		dt: f32 = rl.GetFrameTime()

		for &ball in balls {
			update(&ball, dt)
		}

		rl.BeginDrawing()
		rl.ClearBackground(rl.RAYWHITE)

		for ball in balls {
			rl.DrawCircleV(ball.position, ball.radius, ball.color)

			vText: string = fmt.tprintf("%.2f, %.2f", ball.velocity.x, ball.velocity.y)
			rl.DrawText(
				strings.clone_to_cstring(vText),
				i32(
					ball.position.x - f32(rl.MeasureText(strings.clone_to_cstring(vText), 20) / 2),
				),
				i32(ball.position.y - ball.radius * 2 * 2),
				20,
				rl.BLACK,
			)
		}

		rl.EndDrawing()

		rl.DrawFPS(10, 10)

		if rl.IsKeyPressed(rl.KeyboardKey.ESCAPE) {
			break
		}
	}

	rl.CloseWindow()
}
