# fastkv — In-Memory Key-Value Store (Proyecto profesional)

## Resumen
**fastkv** es un proyecto personal/profesional para construir un _in-memory key-value store_ de alto rendimiento con foco en: diseño de sistemas, concurrencia, networking asíncrono, persistencia simple y herramientas de calidad (tests, benchmarks, CI, sanitizers).  
El objetivo es aprender y demostrar dominio en **C/C++ moderno**, patrones de alta concurrencia y prácticas de ingeniería (vcpkg, CMake, clang-tools, GitHub Actions).

---

## 🎯 Visión — Qué queremos lograr

### **MVP**
- Servidor TCP con comandos: `SET`, `GET`, `DEL`.
- Librería `fastkv` reusable por otros binarios.

### **Nivel profesional**
- Sharding.
- Thread-pool.
- Append-only log para persistencia.
- Benchmarks reproducibles.
- Tests automáticos y CI con sanitizers.
- Integración con Boost.Asio y gRPC (opcional).

### **Resultados visibles (CV/GitHub)**
- Repo limpio y profesional.
- README técnico completo.
- Benchmarks comparativos.
- Badges de CI.
- Issues/PRs organizados.
- Documentación técnica.

---

## 🛠️ Stack tecnológico recomendado

- **Lenguaje:** C++20.
- **Build:** CMake (modern target-based) + Ninja.
- **Paquetería:** vcpkg (manifest mode vía `vcpkg.json`).
- **Compiler:** clang++ (LLVM) o g++ (MinGW).  
  ⚠️ *No mezclar toolchains.*
- **Networking:** Boost.Asio (o standalone Asio).
- **RPC/Serialization:** gRPC + Protobuf (opcional).
- **Testing:** GoogleTest.
- **Benchmarking:** Google Benchmark.
- **Estilo y análisis:** clang-format, clang-tidy, sanitizers (ASan/UBSan/TSan).
- **CI/CD:** GitHub Actions (build matrix, sanitizers, linting).
- **Profiler (futuro):** perf, Tracy, Intel VTune.

---

## 📁 Estructura del repositorio

```bash
fastkv/
├─ CMakeLists.txt
├─ vcpkg.json
├─ include/
│ └─ fastkv/
│ └─ fastkv.hpp
├─ src/
│ ├─ fastkv.cpp
│ └─ main.cpp
├─ tests/
│ └─ test_fastkv.cpp
├─ benchmarks/
│ └─ bench_fastkv.cpp
├─ .vscode/
│ └─ settings.json
├─ .clang-format
├─ .clang-tidy
└─ README.md
```

---

## ⚡ Quickstart — Configuración del entorno (Windows)

Estos pasos asumen que usas **Windows 10+** y trabajarás con **vcpkg**.  
Decide si usarás **clang++ (LLVM)** o **g++ (MinGW)**.  
⚠️ No mezcles librerías compiladas con MSVC si usas MinGW/clang.

---

### 1️⃣ Clonar repositorio

```bash
git clone git@github.com:tuuser/fastkv.git
cd fastkv
```
### 2️⃣ Instalar vcpkg (si aún no lo tienes)

```bash
cd C:\dev
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

Con vcpkg.json en la raíz del proyecto:
```bash
C:\dev\vcpkg\vcpkg.exe install --triplet x64-windows
```
Esto habilita manifest mode.

### 3️⃣ Instalar LLVM (recomendado)

Después de instalar LLVM marcar: Add LLVM to PATH.

Verificar instalación:
```bash
where clang++
where clang-format
where clang-tidy
```

### 4️⃣ Configurar VS Code (.vscode/settings.json)
```json
{
  "cmake.generator": "Ninja",
  "cmake.configureSettings": {
    "CMAKE_TOOLCHAIN_FILE": "C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake"
  },
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "cmake.sourceDirectory": "${workspaceFolder}",
  "C_Cpp.clang_format_style": "file",
  "C_Cpp.codeAnalysis.clangTidy.useBuildPath": true,
  "C_Cpp.codeAnalysis.clangTidy.enabled": true
}
```
### 5️⃣ Configurar compilador en VS Code

Ir a CMake: Scan for Kits.

Elegir un kit con clang++ o g++.

⚠️ Asegúrate de NO usar GTest instalado por Conda.
Si existe, elimina Miniconda del PATH mientras compilas o fuerza GTest_DIR a vcpkg.

### 6️⃣ Configurar y compilar
```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build -j
```
### 7️⃣ Ejecutar tests
```bash
ctest --test-dir build --output-on-failure
```
### 8️⃣ Ejecutar benchmark
```bash
./build/fastkv_bench.exe
```
---

### 🧩 CMake — Puntos clave

enable_testing() + include(GoogleTest) para integración con CTest.

find_package(... CONFIG REQUIRED) para dependencias vcpkg.

Mantener CMAKE_EXPORT_COMPILE_COMMANDS ON.

Opción recomendada:

```C
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)
```
---

### 🧹 Coding style & static checks

clang-format → archivo .clang-format.

clang-tidy → checks:

modernize-*, performance-*, bugprone-*, cppcoreguidelines-*

Sanitizers: ASan + UBSan en CI.

No mezclar runtimes/ABIs entre toolchains.

---

### 📢 Cómo documentar avances (CHANGELOG)

Formato oficial para reportarme avances en el chat:
```md
# CHANGELOG (workspace)
## 2025-11-25 — v0.1-dev (Isok)
### Added
- Inicialización del repo (CMakeLists.txt, vcpkg.json).
- Estructura de carpetas: include/, src/, tests/, benchmarks/.
- Implementación básica de FastKV: header + cpp (put/get/erase).
- main.cpp de prueba.
- .clang-format y .clang-tidy agregados.

### Config
- Instalación y bootstrap de vcpkg en C:\dev\vcpkg.
- VS Code: settings.json configurado con toolchain vcpkg.
- Compilador usado: clang++ (MinGW/LLVM).

### Issues / TODO
- Resolver mezcla de librerías de Miniconda (se quitó del PATH temporalmente).
- Añadir thread-pool y pruebas de concurrencia.
```
---