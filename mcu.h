#ifndef MCU_H
#define MCU_H

#include "digitalcomponent.h"
#include <array>

// شبیه‌ساز یک میکروکنترلر آموزشی ساده (بخش ۷.۲ مستند پروژه).
//
// نکته صادقانه مهم: این شبیه‌سازی یک تراشه تجاری واقعی (مثل 8051 یا AVR) نیست؛
// یک مجموعه‌دستورالعمل کوچک و کاملاً مستندشده مخصوص همین پروژه است که همان
// مفاهیم خواسته‌شده در مستند (PC، رجیستر، RAM، دیکود دستورالعمل، پورت‌های I/O،
// بارگذاری از فایل Hex) را پیاده می‌کند. فرمت دقیق دستورالعمل‌ها پایین همین فایل
// مستند شده تا بشود برایشان فایل .hex دستی نوشت یا از اسمبلر ساده بیرونی تولید کرد.
//
// فرمت دستورالعمل‌ها (۱ بایت opcode + حداکثر ۳ بایت عملوند):
//   0x00                     NOP
//   0x01 port bit            SETB port,bit    (port: 0=P0 1=P1 ، بیت آن پورت را ۱ می‌کند)
//   0x02 port bit            CLR  port,bit    (بیت آن پورت را ۰ می‌کند)
//   0x03 port bit dir        SETDIR port,bit,dir  (dir: 0=Input غیرصفر=Output؛
//                              پیش‌فرض هر پایه Output است - سازگار با فازهای قبلی)
//   0x04 reg port bit        IN   Rreg, port,bit  (مقدار منطقی لحظه‌ای آن پایه را
//                              می‌خواند: High→1, Low→0, Undefined→0 + هشدار در
//                              پنجره گزارشات؛ فقط وقتی آن بیت با SETDIR روی Input
//                              تنظیم شده معنای واقعی دارد)
//   0x05 port reg            OUTPORT port, Rreg  (کل بایت پورت را یک‌جا از رجیستر
//                              می‌نویسد - معادل ۸ بار SETB/CLR پشت‌سرهم اما یک دستور؛
//                              فقط بیت‌هایی که با SETDIR روی Output هستن واقعاً
//                              به پایه اعمال میشن، طبق همون منطق updatePortPins)
//   0x06 reg port             INPORT Rreg, port   (کل بایت پورت را یک‌جا از مقدار
//                              لحظه‌ای هر ۸ پایه می‌خواند - معادل ۸ بار IN پشت‌سرهم)
//   0x10 reg imm8            MOV  Rreg, #imm8
//   0x11 dstReg srcReg       MOV  Rdst, Rsrc
//   0x20 dstReg srcReg       ADD  Rdst, Rsrc      (Rdst = Rdst + Rsrc، سرریز نادیده گرفته می‌شود)
//   0x21 reg imm8            ADD  Rreg, #imm8
//   0x30 addrHi addrLo       JMP  addr16          (پرش مطلق درون ROM)
//   0x31 reg addrHi addrLo   JNZ  Rreg, addr16    (اگه رجیستر ≠ ۰ باشه پرش می‌کند)
//   0x32 reg addrHi addrLo   JZ   Rreg, addr16    (اگه رجیستر = ۰ باشه پرش می‌کند)
//   0xFF                     HALT                 (توقف کامل اجرای برنامه)
// هر دستور ناشناخته هم مثل HALT عمل می‌کند (برای جلوگیری از رفتار تعریف‌نشده).
//
// JNZ/JZ اولین ابزار تصمیم‌گیریِ شرطی این ISA هستن؛ ترکیبشون با IN یعنی برنامه
// واقعاً می‌تونه بر اساس چیزی که از بیرون (مثلاً Keypad) خونده، مسیر اجراش رو
// عوض کنه - نه فقط یک برنامه‌ی خطی از پیش تعیین‌شده.
//
// محدودیت شناخته‌شده: SETB/CLR/SETDIR/IN فقط با بیت/پورتِ *ثابت* (immediate) کار
// می‌کنن، نه بیتِ اندیس‌دار از روی یک رجیستر؛ برای همین یک حلقه‌ی اسکن با شمارنده
// که خودش بیت مورد نظر رو حساب کنه هنوز ممکن نیست. اما OUTPORT به کاربر اجازه
// میده *کل بایت* یک پورت رو از روی مقدار یک رجیستر بنویسه (مثلاً یک الگوی
// یک‌داغ/one-hot که با MOV ساخته شده)، پس برای مواردی مثل «فقط یکی از ۴ ستون
// را در یک لحظه HIGH کن» دیگه نیازی به ۴ دستور SETB/CLR جدا نیست - همین باعث
// شد `keypad_full_scan_demo.hex` (اسکن واقعی هر ۱۶ کلید) توی ۲۵۶ بایت ROM جا
// بشه، برخلاف تلاش قبلی‌اش که فقط با SETB/CLR بیتی حجمش رد می‌شد.
//
// پورت‌های P0/P1 اکنون واقعاً دوطرفه‌اند: هر پایه با SETDIR جداگانه به Input یا
// Output تنظیم می‌شود (پیش‌فرض همه Output، دقیقاً مثل فازهای قبلی - کدهای Hex قدیمی
// بدون تغییر درست کار می‌کنند). یک پایه Input هرگز گره خودش را نمی‌راند (دقیقاً طبق
// Node::resolvedValue فقط پایه‌های Output در تعیین مقدار گره شرکت می‌کنند)؛ IN مقدار
// را از خودِ گره (که پایه‌های Output دیگر متصل به آن رانده‌اند) می‌خواند.
// محدودیت شناخته‌شده: مقاومت Pull-up/Pull-down داخلی مدل نمی‌شود؛ پایه Input بدون
// هیچ درایوری، مثل همیشه Undefined/Floating می‌ماند.
class MCU : public DigitalComponent
{
public:
    MCU();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    // هر گام شبیه‌سازی، دقیقاً یک دستورالعمل اجرا می‌کند (مدل ساده‌شده تک‌سیکل)
    void simulationTick() override;
    void resetSimulation() override;

    // بارگذاری برنامه از یک فایل با فرمت استاندارد Intel HEX (رکوردهای نوع 00 و 01)
    bool loadHexFile(const QString &filePath, QString *errorMessage = nullptr);

protected:
    void populateContextMenu(QMenu *menu) override;

private:
    void executeCurrentInstruction();
    void updatePortPins();
    quint8 fetch(int offset) const;

    static constexpr int kRomSize = 256;
    static constexpr int kRamSize = 256;
    static constexpr int kRegisterCount = 4;

    std::array<quint8, kRomSize> m_rom{};
    std::array<quint8, kRamSize> m_ram{};
    std::array<quint8, kRegisterCount> m_registers{};

    quint16 m_pc = 0;
    quint8 m_portP0 = 0;
    quint8 m_portP1 = 0;
    // هر بیت: ۱=Output (پیش‌فرض، سازگار با فازهای قبلی)، ۰=Input. با SETDIR تغییر می‌کند.
    quint8 m_portP0Dir = 0xFF;
    quint8 m_portP1Dir = 0xFF;
    bool m_halted = false;
};

#endif // MCU_H
