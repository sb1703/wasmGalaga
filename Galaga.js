let lastFrameTime = undefined
let frameDuration = undefined
let ctx = undefined
let canvas = undefined

document.addEventListener("DOMContentLoaded", () => {
  lastFrameTime = 0
  const fps = 60
  frameDuration = 1000 / fps

  // canvas
  canvas = document.getElementById("game-canvas")
  ctx = canvas.getContext("2d")
})

let bg = undefined
let player = undefined
let enemy_spawner = undefined
let particle_spawner = undefined
let checkBulletEnemyCollision = undefined
let checkShipEnemyCollision = undefined

let shipElementWidth = undefined
let shipElementHeight = undefined
let hudElementWidth = undefined
let hudElementHeight = undefined
let healthBarElementWidth = undefined
let healthBarElementHeight = undefined
let shipImageWidth = undefined
let shipImageHeight = undefined
let enemyImageWidth = undefined
let enemyImageHeight = undefined
let explosionFrameWidth = undefined
let explosionFrameHeight = undefined

let wasm_instance = undefined

let canShoot = true
let shootCooldown = 100

const hit_sound = new Audio("Hit.ogg")
const snd_shoot = new Audio("Shoot1.ogg")
snd_shoot.volume = 0.1

// Load all images first
const shipElement = new Image()
const hudElement = new Image()
const healthBarElement = new Image()
const shipImage = new Image()
const enemyImage = new Image()
const anim_explosion = [
  new Image(),
  new Image(),
  new Image(),
  new Image(),
  new Image(),
  new Image(),
]

let imagesLoaded = 0
const totalImages = 11

function checkImagesLoaded() {
  imagesLoaded++
  if (imagesLoaded === totalImages) {
    Module.onRuntimeInitialized = function () {
      console.log("WASM module loaded")
      initializeGame(Module)
    }
  }
}

// initial rendering
shipElement.src = "ship.png"
shipElement.onload = checkImagesLoaded

hudElement.src = "hud.png"
hudElement.onload = checkImagesLoaded

healthBarElement.src = "health_bar.png"
healthBarElement.onload = checkImagesLoaded

shipImage.src = "ship.png"
shipImage.onload = checkImagesLoaded

enemyImage.src = "enemy.png"
enemyImage.onload = checkImagesLoaded

anim_explosion.forEach((image, index) => {
  image.src = `frame${index + 1}.png`
  image.onload = checkImagesLoaded
})

const importObject = {
  env: {
    memory: new WebAssembly.Memory({ initial: 256 }),
    table: new WebAssembly.Table({ initial: 0, element: "anyfunc" }),
    __memory_base: 0,
    __table_base: 0,
    _embind_register_class: function () {
      console.log("_embind_register_class called")
    },
    emscripten_random: function () {
      return Math.random()
    },
  },
  wasi_snapshot_preview1: {},
}

function initializeGame(wasmModule) {
  shipElementWidth = shipElement.naturalWidth * 0.125
  shipElementHeight = shipElement.naturalHeight * 0.125
  healthBarElementWidth = healthBarElement.naturalWidth / 5
  healthBarElementHeight = healthBarElement.naturalHeight / 5
  hudElementWidth = hudElement.naturalWidth * 1.06
  hudElementHeight = hudElement.naturalHeight
  shipImageWidth = shipImage.naturalWidth * 0.05
  shipImageHeight = shipImage.naturalHeight * 0.05
  enemyImageWidth = enemyImage.naturalWidth * 0.05
  enemyImageHeight = enemyImage.naturalHeight * 0.05

  explosionFrameWidth = anim_explosion[0].naturalWidth * 2
  explosionFrameHeight = anim_explosion[0].naturalHeight * 2

  checkBulletEnemyCollision = wasmModule.cwrap(
    "checkBulletEnemyCollisions",
    "boolean",
    ["number", "number", "number"]
  )
  checkShipEnemyCollision = wasmModule.cwrap(
    "checkShipEnemyCollisions",
    "boolean",
    ["number", "number"]
  )

  bg = new wasmModule.Bg()
  player = new wasmModule.Ship(
    shipElementWidth,
    shipElementHeight,
    hudElementHeight,
    healthBarElementHeight
  )
  enemy_spawner = new wasmModule.Enemyspawners(
    enemyImageWidth,
    enemyImageHeight
  )
  particle_spawner = new wasmModule.Particlespawners()
  
  const theme = new Audio("Theme.ogg")
  theme.loop = true
  theme.play()

  handle_events(player)

  // start the game loop
  requestAnimationFrame(gameLoop)
}

function gameLoop(timestamp) {
  const timeSinceLastFrame = timestamp - lastFrameTime

  if (timeSinceLastFrame >= frameDuration) {
    // Update the last frame time to the current timestamp
    lastFrameTime = timestamp

    // handle events

    // update
    bg.update()
    player.update()
    enemy_spawner.update()
    particle_spawner.update()

    const bulletEnemyCollisionResult = checkBulletEnemyCollision(
      player.$$?.ptr,
      enemy_spawner.$$?.ptr,
      particle_spawner.$$?.ptr
    )

    const shipEnemyCollisionResult = checkShipEnemyCollision(
      player.$$?.ptr,
      enemy_spawner.$$?.ptr
    )

    if (bulletEnemyCollisionResult || shipEnemyCollisionResult) {
      hit_sound.play()
    }

    // clear canvas
    ctx.clearRect(0, 0, canvas.width, canvas.height)

    bgDraw()

    // check for game over
    if (!player.is_alive) {
      enemy_spawner.stop_spawning()
      enemy_spawner.clear_enemies()
      ctx.font = 50 + "px Arial"
      ctx.fillStyle = "red"
      ctx.textAlign = "center"

      ctx.fillText("Game Over", 190, 275)
    }

    // console.log(bg.group_star.size())
    // console.log(player.bullet_group.size())
    // console.log(enemy_spawner.enemy_group.size())
    // console.log(particle_spawner.particle_group.size())

    // console.log("HUD lives", player.hud.lives.num_lives)
    // console.log("player lives", player.lives)
    // console.log("player hp", player.hp)

    // render
    bulletDraw()
    enemyDraw()
    particleDraw()
    shipAndHudDraw()
  }

  // Request the next frame
  requestAnimationFrame(gameLoop)
}

function bgDraw() {
  ctx.fillStyle = "rgb(0,0,15)"
  ctx.fillRect(0, 0, canvas.width, canvas.height)

  for(let i=0; i<bg.group_star.size(); i++) {
    const star = bg.group_star.get(i)

    ctx.fillStyle = "white"
    ctx.fillRect(star.x, star.y, star.star_width, star.star_height)
  }
}

function bulletDraw() {
  for(let i=0; i<player.bullet_group.size(); i++) {
    const bullet = player.bullet_group.get(i)

    ctx.fillStyle = "white"
    ctx.fillRect(bullet.x, bullet.y, bullet.bullet_width, bullet.bullet_height)
  }
}

function enemyDraw() {
  for(let i=0; i<enemy_spawner.enemy_group.size(); i++) {
    const enemy = enemy_spawner.enemy_group.get(i)

    if (
      enemy.is_destroyed &&
      enemy.anim_index <= 5
    ) {
      ctx.drawImage(
        anim_explosion[enemy.anim_index],
        enemy.x,
        enemy.y,
        explosionFrameWidth,
        explosionFrameHeight
      )
    } else {
      ctx.drawImage(
        enemyImage,
        enemy.x,
        enemy.y,
        enemyImageWidth,
        enemyImageHeight
      )
    }
  }
}

function particleDraw() {
  for(let i=0; i<particle_spawner.particle_group.size(); i++) {
    const particle = particle_spawner.particle_group.get(i)

    ctx.fillStyle = "white"
    ctx.fillRect(
      particle.x,
      particle.y,
      particle.particle_width,
      particle.particle_height
    )
  }
}

function shipAndHudDraw() {
  if (player.is_alive) {
    ctx.drawImage(
      shipElement,
      player.x,
      player.y,
      shipElementWidth,
      shipElementHeight
    )
  }

  ctx.drawImage(
    hudElement,
    player.hud.x,
    player.hud.y,
    hudElementWidth,
    hudElementHeight
  )

  ctx.drawImage(
    healthBarElement,
    player.hud.health_bar.x,
    player.hud.health_bar.y,
    healthBarElementWidth *
      (player.hud.health_bar.hp / player.hud.health_bar.max_hp),
    healthBarElementHeight
  )

  ctx.font = player.hud.score.font_size + "px Arial"
  ctx.fillStyle = "white"
  ctx.fillText(
    "Score: " + player.hud.score.value,
    player.hud.score.x,
    player.hud.score.y
  )

  ctx.drawImage(
    shipImage,
    player.hud.lives.x,
    player.hud.lives.y,
    shipImageWidth,
    shipImageHeight
  )

  ctx.font = player.hud.lives.font_size + "px Arial"
  ctx.fillStyle = "white"
  ctx.fillText(
    "x" + player.hud.lives.num_lives,
    player.hud.lives.x + shipImageWidth + 5,
    player.hud.score.y
  )
}

function handle_events(actor) {
  document.addEventListener("keydown", (e) => {
    if (e.key === "a") {
      actor.left()
    } else if (e.key === "d") {
      actor.right()
    } else if (e.key === " " && canShoot) {
      if (actor.is_alive) {
        snd_shoot.play()
        actor.shoot()
      }
      canShoot = false
      setTimeout(() => {
        canShoot = true
      }, shootCooldown)
    }
  })

  document.addEventListener("keyup", (e) => {
    if (e.key === "a" || e.key === "d") {
      actor.zero_x()
    }
  })
}
