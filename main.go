package main

import (
	"fmt"
	"image/color"
	"log"
	"os"

	"github.com/hajimehoshi/ebiten/v2"
	"github.com/hajimehoshi/ebiten/v2/inpututil"
	"github.com/hajimehoshi/ebiten/v2/text"
	"golang.org/x/image/font/basicfont"
)

const (
	screenWidth  = 640
	screenHeight = 480
)

type Settings struct {
	gravity      float64
	bounceFactor float64
	ballSpeed    float64
	ballSize     float64
	ballColor    color.Color
}

type Ball struct {
	x, y      float64
	velocityX float64
	velocityY float64
	radius    float64
	color     color.Color
}

type GameState int

const (
	StateMainMenu GameState = iota
	StateSettings
	StatePlaying
)

type Game struct {
	state        GameState
	ball         *Ball
	settings     Settings
	menuItem     int
	settingsItem int
}

func NewGame() *Game {
	defaultSettings := Settings{
		gravity:      0.5,
		bounceFactor: 0.7,
		ballSpeed:    3.0,
		ballSize:     20.0,
		ballColor:    color.RGBA{255, 0, 0, 255},
	}

	return &Game{
		state:    StateMainMenu,
		settings: defaultSettings,
	}
}

func NewBall(settings Settings) *Ball {
	return &Ball{
		x:         float64(screenWidth) / 2,
		y:         float64(screenHeight) / 4,
		velocityX: settings.ballSpeed,
		velocityY: 0,
		radius:    settings.ballSize,
		color:     settings.ballColor,
	}
}

func (g *Game) Update() error {
	switch g.state {
	case StateMainMenu:
		g.updateMainMenu()
	case StateSettings:
		g.updateSettings()
	case StatePlaying:
		g.updateGame()
	}
	return nil
}

func (g *Game) updateMainMenu() {
	if inpututil.IsKeyJustPressed(ebiten.KeyDown) {
		g.menuItem = (g.menuItem + 1) % 3
	}
	if inpututil.IsKeyJustPressed(ebiten.KeyUp) {
		g.menuItem = (g.menuItem - 1 + 3) % 3
	}
	if inpututil.IsKeyJustPressed(ebiten.KeyEnter) {
		switch g.menuItem {
		case 0: // Start
			g.ball = NewBall(g.settings)
			g.state = StatePlaying
		case 1: // Settings
			g.state = StateSettings
		case 2: // Quit
			os.Exit(0)
		}
	}
}

func (g *Game) updateSettings() {
	if inpututil.IsKeyJustPressed(ebiten.KeyDown) {
		g.settingsItem = (g.settingsItem + 1) % 5
	}
	if inpututil.IsKeyJustPressed(ebiten.KeyUp) {
		g.settingsItem = (g.settingsItem - 1 + 5) % 5
	}
	if inpututil.IsKeyJustPressed(ebiten.KeyEscape) {
		g.state = StateMainMenu
	}

	// Adjust selected setting
	if inpututil.IsKeyJustPressed(ebiten.KeyRight) {
		switch g.settingsItem {
		case 0:
			g.settings.gravity += 0.1
		case 1:
			g.settings.bounceFactor += 0.1
		case 2:
			g.settings.ballSpeed += 0.5
		case 3:
			g.settings.ballSize += 5
		}
	}
	if inpututil.IsKeyJustPressed(ebiten.KeyLeft) {
		switch g.settingsItem {
		case 0:
			g.settings.gravity -= 0.1
		case 1:
			g.settings.bounceFactor -= 0.1
		case 2:
			g.settings.ballSpeed -= 0.5
		case 3:
			g.settings.ballSize -= 5
		}
	}
}

func (g *Game) updateGame() {
	if inpututil.IsKeyJustPressed(ebiten.KeyEscape) {
		g.state = StateMainMenu
		return
	}

	// Apply gravity
	g.ball.velocityY += g.settings.gravity

	// Update position
	g.ball.x += g.ball.velocityX
	g.ball.y += g.ball.velocityY

	// Bounce off walls
	if g.ball.x-g.ball.radius <= 0 || g.ball.x+g.ball.radius >= screenWidth {
		g.ball.velocityX *= -g.settings.bounceFactor
		if g.ball.x < g.ball.radius {
			g.ball.x = g.ball.radius
		}
		if g.ball.x > screenWidth-g.ball.radius {
			g.ball.x = screenWidth - g.ball.radius
		}
	}

	// Bounce off floor and ceiling
	if g.ball.y-g.ball.radius <= 0 || g.ball.y+g.ball.radius >= screenHeight {
		g.ball.velocityY *= -g.settings.bounceFactor
		if g.ball.y < g.ball.radius {
			g.ball.y = g.ball.radius
		}
		if g.ball.y > screenHeight-g.ball.radius {
			g.ball.y = screenHeight - g.ball.radius
		}
	}
}

func (g *Game) Draw(screen *ebiten.Image) {
	// Clear the screen
	screen.Fill(color.Black)

	switch g.state {
	case StateMainMenu:
		g.drawMainMenu(screen)
	case StateSettings:
		g.drawSettings(screen)
	case StatePlaying:
		g.drawGame(screen)
	}
}

func (g *Game) drawMainMenu(screen *ebiten.Image) {
	menuItems := []string{"Start", "Settings", "Quit"}
	for i, item := range menuItems {
		textColor := color.White
		if i == g.menuItem {
			textColor = color.Black
		}
		text.Draw(screen, item, basicfont.Face7x13,
			screenWidth/2-20,
			screenHeight/2-30+i*30,
			textColor)
	}
	text.Draw(screen, "Use Arrow Keys to Navigate, Enter to Select",
		basicfont.Face7x13,
		10, screenHeight-20,
		color.White)
}

func (g *Game) drawSettings(screen *ebiten.Image) {
	settings := []string{
		fmt.Sprintf("Gravity: %.1f", g.settings.gravity),
		fmt.Sprintf("Bounce Factor: %.1f", g.settings.bounceFactor),
		fmt.Sprintf("Ball Speed: %.1f", g.settings.ballSpeed),
		fmt.Sprintf("Ball Size: %.0f", g.settings.ballSize),
		"Back to Main Menu",
	}

	for i, setting := range settings {
		textColor := color.White
		if i == g.settingsItem {
			textColor = color.White
		}
		text.Draw(screen, setting, basicfont.Face7x13,
			screenWidth/2-50,
			screenHeight/2-60+i*30,
			textColor)
	}
	text.Draw(screen, "Arrow Keys to Navigate/Adjust, ESC to Return",
		basicfont.Face7x13,
		10, screenHeight-20,
		color.White)
}

func (g *Game) drawGame(screen *ebiten.Image) {
	// Draw the ball
	DrawCircle(screen, int(g.ball.x), int(g.ball.y), int(g.ball.radius), g.ball.color)

	// Draw game instructions
	text.Draw(screen, "Press ESC to Return to Menu",
		basicfont.Face7x13,
		10, screenHeight-20,
		color.White)
}

func (g *Game) Layout(outsideWidth, outsideHeight int) (int, int) {
	return screenWidth, screenHeight
}

// DrawCircle draws a filled circle using the midpoint circle algorithm
func DrawCircle(screen *ebiten.Image, x0, y0, radius int, c color.Color) {
	for y := -radius; y <= radius; y++ {
		for x := -radius; x <= radius; x++ {
			if x*x+y*y <= radius*radius {
				screen.Set(x0+x, y0+y, c)
			}
		}
	}
}

func main() {
	ebiten.SetWindowSize(screenWidth, screenHeight)
	ebiten.SetWindowTitle("Physics Simulation")

	game := NewGame()

	if err := ebiten.RunGame(game); err != nil {
		log.Fatal(err)
	}
}
