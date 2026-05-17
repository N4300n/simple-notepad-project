# Notepad Project (Qt 6)

## 📌 Overview

This project is a fully functional **Notepad-like text editor** developed using **C++ and Qt 6 Widgets**.

It extends a basic text editor into a more advanced WordPad-style application with:

- File management system
- Rich text formatting
- Spell checking system
- Word analysis tools
- Exception handling system
- GUI dialogs for search and statistics

---

<img width="435" height="554" alt="image" src="https://github.com/user-attachments/assets/9d1b2856-bfda-4660-8780-cacd4f40b3a6" />


---

## 🧠 Architecture

The application is built using **Qt Widgets** and follows modular OOP design.

### Main Components:

### 1. main_window
Core class of the application:
- Handles menus and toolbars
- Controls QTextEdit editor
- Manages file operations
- Connects all features together

---

### 2. spell_checker
Responsible for:
- Loading dictionary of valid words
- Checking spelling correctness
- Providing suggestions for misspelled words

Dictionary is loaded from internal resource file (data words list).

---

### 3. highlighter (QSyntaxHighlighter)
- Performs real-time spell checking
- Highlights incorrect words with red underline
- Works automatically on text change

---

### 4. text_transform
Provides text case transformations:
- Uppercase
- Lowercase
- Capitalize
- Sentence case
- Swap case

---

### 5. notepad_exception
Custom exception system:
- file_not_found_exception
- file_read_exception
- file_write_exception

Used for safe file operations.

---

### 6. test_exceptions
A small test module used to verify exception behavior during development.

---

## ⚙️ Features

### 📂 File System
- New file
- Open file
- Save / Save As
- Exit

With exception handling for file errors.

---

### ✏️ Text Editing
- Undo / Redo
- Cut / Copy / Paste
- Select All

---

### 🎨 Text Formatting
- Bold
- Italic
- Underline
- Font selection
- Text color

Toolbar includes icons for formatting actions.

---

### 🔍 Find & Replace
- Find next occurrence
- Replace current
- Replace all
- Case-sensitive search option

Implemented using QDialog UI.

---

### 📊 Word Frequency Tool
- Counts word occurrences
- Removes punctuation
- Displays results in QTableWidget
- Sorted by frequency

---

### 🧠 Spell Checker System
- Loads dictionary on startup
- Real-time spell highlighting
- Context menu suggestions (right-click)
- Manual "Check Spelling" action

---

### 📍 Status Bar
Displays:
- Word count
- Line count
- Cursor position (Line / Column)

---

## 🖼 UI Design

The UI is built using Qt Widgets:

- QMainWindow (main window)
- QTextEdit (editor)
- QMenuBar (menus)
- QToolBar (formatting)
- QDialog (find/replace, word frequency)

UI files:
- find_replace_dialog.ui
- word_frequency_dialog.ui

---

## ⭐ Optional Features Implemented

### ✔ Cursor Position Indicator
Shows current line and column in status bar.

### ✔ Font Dialog
Allows selecting font for text.

### ✔ Text Color Picker
Allows changing text color.

### ✔ Word Frequency Tool
Analyzes text and shows frequency table.

### ✔ Spell Checker
Real-time underline + suggestions.

---
