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
	g: f32 = 9.81 * 50 // gravity acceleration
	d: f32 = 0.99 / 5 // drag (damping) coefficient
	r: f32 = 0.75 // restitution (bounce factor)
	ty: f32 = 32 // bounce threshold
	f: f32 = 0.9 // friction (horizontal) for the ground
	tx: f32 = 5 // horizontal friction threshold

	ball.velocity.y = ball.velocity.y + g * dt
	ball.velocity.x = ball.velocity.x * (1 - d * dt)
	ball.velocity.y = ball.velocity.y * (1 - d * dt)

	if (ball.position.x - ball.radius) <= 0 {
		ball.position.x = ball.radius
		ball.velocity.x *= -r
	}
	if (ball.position.x + ball.radius) >= f32(rl.GetScreenWidth()) {
		ball.position.x = f32(rl.GetScreenWidth()) - ball.radius
		ball.velocity.x *= -r
	}

	if (ball.position.y - ball.radius) <= 0 {
		ball.position.y = ball.radius
		ball.velocity.y *= -r
	}
	if (ball.position.y + ball.radius) >= f32(rl.GetScreenHeight()) {
		ball.position.y = f32(rl.GetScreenHeight()) - ball.radius
		ball.velocity.y *= -r

		if abs(ball.velocity.y) < ty {
			ball.velocity.y = 0

			ball.velocity.x = ball.velocity.x * f
			if abs(ball.velocity.x) < tx {
				ball.velocity.x = 0
			}
		}
	}

	ball.position = ball.position + ball.velocity * dt
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
