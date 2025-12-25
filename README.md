<p align="center">
  <img src="icon.iconset/icon_256x256.png" alt="Dev Planner Logo" width="128" height="128">
</p>

<h1 align="center">Dev Planner</h1>

<p align="center">
  <strong>🚀 AI-Powered Task & Project Planning Tool</strong>
</p>

<p align="center">
  <a href="README.md">🇬🇧 English</a> •
  <a href="README.ru.md">🇷🇺 Русский</a>
</p>

<p align="center">
  <a href="https://github.com/prisset/Dev-Planner/releases/latest">
    <img src="https://img.shields.io/github/v/release/prisset/Dev-Planner?style=for-the-badge&color=00ff9d" alt="Release">
  </a>
  <a href="https://github.com/prisset/Dev-Planner/actions">
    <img src="https://img.shields.io/github/actions/workflow/status/prisset/Dev-Planner/build.yml?style=for-the-badge&color=d900ff" alt="Build">
  </a>
  <a href="https://github.com/prisset/Dev-Planner/blob/main/LICENSE">
    <img src="https://img.shields.io/github/license/prisset/Dev-Planner?style=for-the-badge&color=00ffff" alt="License">
  </a>
</p>

<p align="center">
  <a href="#-features">Features</a> •
  <a href="#-installation">Installation</a> •
  <a href="#-usage">Usage</a> •
  <a href="#-ai-commands">AI Commands</a> •
  <a href="#-building">Building</a>
</p>

---

## ✨ Features

<table>
<tr>
<td width="50%">

### 🎯 Visual Task Management
- Create and organize tasks on an infinite canvas
- Connect tasks with visual relationships
- Drag and drop interface
- Zoom and pan navigation

</td>
<td width="50%">

### 🤖 AI Co-Pilot
- Natural language task creation
- Bulk operations via AI
- Smart task organization
- Automated workflows

</td>
</tr>
<tr>
<td width="50%">

### 📝 Note Mode
- Quick notes without titles
- Focus on content
- Compact view

</td>
<td width="50%">

### 💾 Project Management
- Multiple projects support
- Auto-save functionality
- JSON export/import

</td>
</tr>
</table>

## 📥 Installation

### Windows
Download and run **`Dev.Planner.Setup.exe`** from the [latest release](https://github.com/prisset/Dev-Planner/releases/latest).

### macOS
Download **`Dev.Planner.macOS.dmg`** from the [latest release](https://github.com/prisset/Dev-Planner/releases/latest), open it and drag Dev Planner to Applications.

## 🎮 Usage

### Basic Controls
| Action | Control |
|--------|---------|
| Create Task | Double-click on canvas |
| Move Task | Drag with mouse |
| Connect Tasks | Right-click → Connect, then click target |
| Delete Task | Click × button |
| Zoom | Mouse wheel or +/- buttons |
| Pan | Middle mouse button or Shift+drag |

### Status Colors
- 🔴 **Todo** - Not started
- 🟡 **Progress** - In progress  
- 🟢 **Done** - Completed
- ⚫ **None** - No status

## 🤖 AI Commands

The AI Co-Pilot understands natural language. Examples:

```
"Create 5 tasks for website development"
"Make all tasks green"
"Connect task 1 to task 3"
"Delete tasks 2 and 4"
"Clear everything"
```

### Available Actions
| Command | Description |
|---------|-------------|
| `create_task` | Create a single task |
| `create_tasks_chain` | Create connected tasks |
| `set_status` | Change task status |
| `set_many_status` | Change multiple tasks status |
| `connect` | Connect two tasks |
| `connect_many` | Create multiple connections |
| `rename` | Rename a task |
| `delete` | Delete a task |
| `clear_all` | Remove all tasks |

## 🔧 Building

### Requirements
- Qt 6.6+
- CMake 3.20+
- C++17 compiler

### Build Steps

```bash
cd cpp
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Windows (MSVC)
```powershell
cd cpp
mkdir build && cd build
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
nmake
windeployqt DevPlanner.exe
```

## 📁 Project Structure

```
cpp/
├── src/
│   ├── ai/                 # AI action system
│   │   ├── actions/        # Individual action handlers
│   │   ├── ai_action.hpp   # Base action interface
│   │   └── ai_action_registry.cpp
│   ├── core/              # Core utilities
│   └── ui/                # UI components
│       ├── main_window.cpp
│       ├── node_canvas.cpp
│       ├── task_node.cpp
│       └── ai_chat_panel.cpp
└── CMakeLists.txt
```

## 📄 License

MIT License - see [LICENSE](LICENSE) for details.

---

<p align="center">
  Made with ❤️ by <a href="https://github.com/prisset">prisset</a>
</p>
