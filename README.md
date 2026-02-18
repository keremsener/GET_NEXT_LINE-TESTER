# GNL TESTER

*Created by **ksener** (42 Kocaeli)*

## Description

This tester is designed to push your `get_next_line` function to its limits. It checks not only for basic functionality but also for **memory leaks, buffer stress, static variable management, and undefined behaviors** (like reading from closed file descriptors or permission-denied files).

**This tester includes ALL TEST almost BUT this tester is NOT REAL moulinette. So, if you passed this project this does not guarantee. If you would like to check your project, okay, you can use it.**
## Features

- **Visual Feedback:** Clear Green (OK) and Red (FAIL) indicators.
- **🛡️ Hardcore Edge Cases:** Tests `chmod 000` files, closed FDs, and empty files.
- **💥 Buffer Stress Test:** Switches between very short and very long lines (2000+ chars) to test memory management.
- **🕵️ Bonus Detection:** Automatically detects if your static variable handles multiple file descriptors correctly.
- **⚡ Custom Buffer Size:** Easily test with `BUFFER_SIZE=1` or `BUFFER_SIZE=10000`.

## 📂 How to Use

### 1. Setup
Place the `tester.c` (or `main.c`) file in the same directory as your `get_next_line` files.

### 2. Compile
You need to compile the tester along with your source files. Don't forget to define the `BUFFER_SIZE`!

**For Mandatory Part:**
```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 main.c get_next_line.c get_next_line_utils.c -o gnl_tester
```

**For Bonus Part:**
```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 main.c get_next_line_bonus.c get_next_line_utils_bonus.c -o gnl_tester
```

**Run**
```bash
./gnl_tester
```

**Manual STDIN Test**
```bash
echo "Hello 42" | ./gnl_tester manual
```

## ⚠️ Understanding the Output

- ✅ OK: Your function returned the correct line.

- ❌ FAIL: Your function returned the wrong line, NULL when it shouldn't have, or crashed.

`⚠️ WARNING (Bonus): If you see "Mixed content detected" (e.g., getting text from File 1 while reading File 2), it means your static variable is not handling multiple FDs. This is normal if you only did the Mandatory part.`