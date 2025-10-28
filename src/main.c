#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s <model_file> [msaa]\n", argv[0]);
    printf("Options:\n");
    printf("  msaa - Enable 4x MSAA antialiasing\n");
    return 1;
  }

  char *filename = argv[1];
  char *lastSlash = strrchr(argv[1], '/');
  if (lastSlash != NULL) {
    filename = lastSlash + 1;
  }

  bool enableMsaa = (argc > 2 && strcmp(argv[2], "msaa") == 0);

  if (enableMsaa) {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
  }

  InitWindow(800, 600, "Model Viewer - AO Shader");
  SetTargetFPS(60);

  Camera3D camera = {0};
  camera.position = (Vector3){5.0f, 5.0f, 5.0f};
  camera.target = (Vector3){0.0f, 0.0f, 0.0f};
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  Model model = LoadModel(argv[1]);

  bool modelLoaded = (model.meshCount > 0 && model.meshes != NULL);

  if (!modelLoaded) {
    printf("ERROR: Failed to load model: %s\n", argv[1]);
    printf("Supported formats: .obj, .gltf, .glb, .iqm, .vox\n");
  }

  // Load ambient occlusion shader
  Shader aoShader = {0};
  bool shaderLoaded = false;
  
  if (modelLoaded) {
    aoShader = LoadShader("shaders/ao.vs", "shaders/ao.fs");
    
    if (aoShader.id > 0) {
      shaderLoaded = true;
      
      // Apply shader to all materials
      for (int i = 0; i < model.materialCount; i++) {
        model.materials[i].shader = aoShader;
      }
      
      // Get shader locations
      aoShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(aoShader, "viewPos");
      aoShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(aoShader, "matModel");
      
      printf("Ambient Occlusion shader loaded successfully!\n");
    } else {
      printf("WARNING: Failed to load AO shader, using default rendering\n");
    }
  }

  bool showGrid = true;
  float modelScale = 1.0f;

  while (!WindowShouldClose()) {
    // Controls
    if (modelLoaded) {
      if (IsKeyPressed(KEY_F))
        ToggleFullscreen();
      
      if (IsKeyPressed(KEY_R)) {
        camera.position = (Vector3){5.0f, 5.0f, 5.0f};
        camera.target = (Vector3){0.0f, 0.0f, 0.0f};
        modelScale = 1.0f;
      }
      
      if (IsKeyPressed(KEY_G))
        showGrid = !showGrid;
      
      if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD))
        modelScale += 0.1f;
      if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT))
        modelScale = (modelScale > 0.1f) ? modelScale - 0.1f : 0.1f;

      UpdateCamera(&camera, CAMERA_ORBITAL);
      
      // Update shader with camera position
      if (shaderLoaded) {
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(aoShader, aoShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
      }
    }

    // Drawing
    BeginDrawing();
    ClearBackground((Color){30, 35, 40, 255});

    if (modelLoaded) {
      BeginMode3D(camera);

      DrawModel(model, (Vector3){0.0f, 0.0f, 0.0f}, modelScale, WHITE);
      
      if (showGrid)
        DrawGrid(10, 1.0f);

      EndMode3D();

      // UI
      DrawText(filename, 10, 10, 20, ORANGE);
      DrawText(shaderLoaded ? "AO Shader: ON" : "AO Shader: FAILED", 10, 35, 12, 
               shaderLoaded ? GREEN : RED);
      DrawText("Controls:", 10, 55, 12, LIGHTGRAY);
      DrawText("  Scroll: Zoom", 10, 70, 10, LIGHTGRAY);
      DrawText("  +/-: Scale model", 10, 85, 10, LIGHTGRAY);
      DrawText("  R: Reset view", 10, 100, 10, LIGHTGRAY);
      DrawText("  G: Toggle grid", 10, 115, 10, LIGHTGRAY);
      DrawText("  F: Fullscreen", 10, 130, 10, LIGHTGRAY);
      
      DrawText(TextFormat("Scale: %.1f", modelScale), 10, 155, 10, YELLOW);
      
    } else {
      DrawText("Failed to load model!", 10, 10, 24, RED);
      DrawText(argv[1], 10, 40, 12, WHITE);
      DrawText("Supported formats: .obj, .gltf, .glb, .iqm, .vox", 10, 60, 12,
               LIGHTGRAY);
    }
    
    DrawFPS(10, GetScreenHeight() - 25);

    EndDrawing();
  }

  if (modelLoaded) {
    if (shaderLoaded) {
      UnloadShader(aoShader);
    }
    UnloadModel(model);
  }

  CloseWindow();

  return 0;
}
