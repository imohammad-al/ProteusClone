
راهنمای سریع راه‌اندازی و اجرای پروژه Qt6 در CLion

---

## 🛠️ ۱. پیش‌نیازها
1. **CLion IDE** (نسخه 2023.2 به بالا)
2. **Qt6** همراه با **MinGW 64-bit** (دریافت از طریق Qt Online Installer)

---

## ⚙️ ۲. پیکربندی CLion

### ۱. تنظیم Toolchain (کامپایلر و دیباگر)
* به مسیر **Settings -> Build, Execution, Deployment -> Toolchains** بروید و یک **MinGW** جدید بسازید.
* کادر **Environment file** را **خالی** بگذارید.
* در کادر **Toolset** آدرس پوشه MinGW کیوت را بدهید (مثلاً `A:\Qt\Tools\mingw1310_64`).
* **مهم:** مطمئن شوید کامپایلرهای **C Compiler** و **C++ Compiler** حتماً روی فایل‌های `gcc.exe` و `g++.exe` مربوط به **MinGWِ خودِ Qt** تنظیم شده باشند (نه کامپایلر متفرقه سیستم).

### ۲. تنظیمات CMake Profile
* به مسیر **Settings -> Build, Execution, Deployment -> CMake** بروید.
* یک Profile (مثلاً `QT debug`) بسازید و **Toolchain** آن را روی MinGW مرحله قبل بگذارید.
* **مهم:** در کادر **Debugger** حتماً دیباگر را روی گزینه مربوط به این **CMake / Toolchain جدید** تنظیم کنید.
* در کادر **CMake options** مسیر Qt6 را وارد کنید:
  ```text
  -DCMAKE_PREFIX_PATH="A:/Qt/6.8.3/mingw_64"

```

### ۳. رفع خطای اجرا و نبود DLL (کد 0xC0000135)

* از منوی بالا گزینه **Edit Configurations...** را بزنید.
* در کادر **Environment variables** مسیر `bin` کیوت را اضافه کنید:
```text
PATH=A:\Qt\6.8.3\mingw_64\bin;%PATH%

```



---

## 📝 ۳. نکته مهم در `CMakeLists.txt`

برای جلوگیری از خطای `No such file or directory` در فایل‌های `.ui` و Headerها، حتماً دستور زیر را در فایل CMake قرار دهید:

```cmake
target_include_directories(ProteusClone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

```

---

## 🚀 ۴. اجرا

پروفایل را روی **`QT debug`** بگذارید و روی دکمه **Run** (یا `Shift + F10`) کلیک کنید.

```

```
