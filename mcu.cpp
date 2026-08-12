#include "mcu.h"
#include "pin.h"
#include "node.h"

#include <QPainter>
#include <QFile>
#include <QTextStream>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include "sim/simulationlogger.h"

MCU::MCU() {
    setComponentType("MCU");
    setCategory("Advanced");

    // پورت P0: پایه‌های اندیس 0-7 (سمت راست)
    for (int i = 0; i < 8; ++i) {
        Pin *p = new Pin(this);
        p->setPos(45, -35 + i * 10);
        p->setDirection(PinDirection::Output);
        addPin(p);
    }

    // پورت P1: پایه‌های اندیس 8-15 (پایین)
    for (int i = 0; i < 8; ++i) {
        Pin *p = new Pin(this);
        p->setPos(-35 + i * 10, 45);
        p->setDirection(PinDirection::Output);
        addPin(p);
    }

    resetSimulation();
}

QRectF MCU::boundingRect() const { return QRectF(-45, -45, 100, 100); }
QPainterPath MCU::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }

Component* MCU::clone() const { return new MCU(); }

void MCU::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QColor(50, 50, 60));
    painter->drawRect(QRectF(-40, -35, 80, 75));
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < 8; ++i)
        painter->drawLine(QPointF(40, -35 + i * 10), QPointF(45, -35 + i * 10));
    for (int i = 0; i < 8; ++i)
        painter->drawLine(QPointF(-35 + i * 10, 40), QPointF(-35 + i * 10, 45));

    painter->setPen(Qt::white);
    painter->drawText(QRectF(-38, -30, 76, 20), Qt::AlignCenter, "MCU");
    painter->drawText(QRectF(-38, -8, 76, 20), Qt::AlignCenter,
                       QString("PC:%1").arg(m_pc, 3, 16, QChar('0')).toUpper());

    painter->setPen(Qt::black);
    painter->drawText(-38, -40, name());
    paintSelectionOverlay(painter, option);
}

void MCU::populateContextMenu(QMenu *menu)
{
    QAction *loadAction = menu->addAction(QObject::tr("Load HEX File..."));
    QObject::connect(loadAction, &QAction::triggered, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(
            nullptr, QObject::tr("Load HEX File"), QString(),
            QObject::tr("Intel HEX Files (*.hex)"));

        if (path.isEmpty())
            return;

        QString error;
        if (loadHexFile(path, &error)) {
            SimulationLogger::instance().log(LogLevel::Info,
                QObject::tr("برنامه با موفقیت در MCU بارگذاری شد: %1").arg(path));
        } else {
            SimulationLogger::instance().log(LogLevel::Error,
                QObject::tr("خطا در بارگذاری فایل Hex: %1").arg(error));
        }
    });
}

bool MCU::loadHexFile(const QString &filePath, QString *errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = QObject::tr("امکان باز کردن فایل hex وجود ندارد.");
        return false;
    }

    m_rom.fill(0);

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        if (!line.startsWith(':')) {
            if (errorMessage) *errorMessage = QObject::tr("فرمت فایل hex نامعتبر است (خط باید با : شروع شود).");
            return false;
        }

        line = line.mid(1);
        if (line.size() < 10) {
            if (errorMessage) *errorMessage = QObject::tr("خط hex خیلی کوتاه است.");
            return false;
        }

        bool ok = false;
        const int byteCount = line.mid(0, 2).toInt(&ok, 16);
        const int address = ok ? line.mid(2, 4).toInt(&ok, 16) : 0;
        const int recordType = ok ? line.mid(6, 2).toInt(&ok, 16) : 0;

        if (!ok) {
            if (errorMessage) *errorMessage = QObject::tr("سرآیند رکورد hex نامعتبر است.");
            return false;
        }

        if (recordType == 1) // EOF
            break;

        if (recordType == 0) { // Data record
            if (line.size() < 8 + byteCount * 2) {
                if (errorMessage) *errorMessage = QObject::tr("طول داده رکورد hex با byteCount همخوانی ندارد.");
                return false;
            }

            for (int i = 0; i < byteCount; ++i) {
                const int byteVal = line.mid(8 + i * 2, 2).toInt(&ok, 16);
                if (!ok) {
                    if (errorMessage) *errorMessage = QObject::tr("داده hex نامعتبر است.");
                    return false;
                }
                if (address + i < kRomSize)
                    m_rom[address + i] = static_cast<quint8>(byteVal);
            }
        }
        // رکوردهای دیگر (مثل Extended Linear Address) در این نسخه ساده نادیده گرفته می‌شوند.
        // نکته: checksum انتهای هر خط عمداً اعتبارسنجی نمی‌شود (ساده‌سازی آگاهانه این فاز).
    }

    resetSimulation();
    return true;
}

quint8 MCU::fetch(int offset) const
{
    const int addr = m_pc + offset;
    return (addr >= 0 && addr < kRomSize) ? m_rom[addr] : 0;
}

void MCU::simulationTick()
{
    if (m_halted)
        return;

    executeCurrentInstruction();
    updatePortPins();
}

void MCU::executeCurrentInstruction()
{
    if (m_pc >= kRomSize) {
        m_halted = true;
        return;
    }

    const quint8 opcode = m_rom[m_pc];

    switch (opcode) {
    case 0x00: // NOP
        m_pc += 1;
        break;

    case 0x01: { // SETB port, bit
        const quint8 port = fetch(1);
        const quint8 bit = fetch(2);
        if (bit < 8) {
            if (port == 0) m_portP0 |= static_cast<quint8>(1u << bit);
            else            m_portP1 |= static_cast<quint8>(1u << bit);
        }
        m_pc += 3;
        break;
    }

    case 0x02: { // CLR port, bit
        const quint8 port = fetch(1);
        const quint8 bit = fetch(2);
        if (bit < 8) {
            if (port == 0) m_portP0 &= static_cast<quint8>(~(1u << bit));
            else            m_portP1 &= static_cast<quint8>(~(1u << bit));
        }
        m_pc += 3;
        break;
    }

    case 0x03: { // SETDIR port, bit, dir  (dir: 0=Input, غیرصفر=Output)
        const quint8 port = fetch(1);
        const quint8 bit = fetch(2);
        const quint8 dir = fetch(3);
        if (bit < 8) {
            quint8 &dirReg = (port == 0) ? m_portP0Dir : m_portP1Dir;
            if (dir == 0)
                dirReg &= static_cast<quint8>(~(1u << bit));
            else
                dirReg |= static_cast<quint8>(1u << bit);
        }
        m_pc += 4;
        break;
    }

    case 0x04: { // IN Rreg, port, bit  (مقدار لحظه‌ای گره آن پایه را می‌خواند)
        const quint8 reg = fetch(1);
        const quint8 port = fetch(2);
        const quint8 bit = fetch(3);
        if (reg < kRegisterCount && bit < 8) {
            Pin *p = (port == 0) ? pin(bit) : pin(8 + bit);
            LogicValue value = (p && p->node()) ? p->node()->resolvedValue() : LogicValue::Undefined;
            if (value == LogicValue::Undefined) {
                SimulationLogger::instance().log(LogLevel::Warning,
                    QObject::tr("MCU: خواندن پایه شناور/نامشخص P%1.%2 - مقدار ۰ در نظر گرفته شد.")
                        .arg(port).arg(bit));
            }
            m_registers[reg] = (value == LogicValue::High) ? 1 : 0;
        }
        m_pc += 4;
        break;
    }

    case 0x05: { // OUTPORT port, Rreg  (کل بایت پورت را یک‌جا از رجیستر می‌نویسد)
        const quint8 port = fetch(1);
        const quint8 reg = fetch(2);
        if (reg < kRegisterCount) {
            if (port == 0) m_portP0 = m_registers[reg];
            else            m_portP1 = m_registers[reg];
        }
        m_pc += 3;
        break;
    }

    case 0x06: { // INPORT Rreg, port  (کل بایت پورت را یک‌جا از پایه‌ها می‌خواند)
        const quint8 reg = fetch(1);
        const quint8 port = fetch(2);
        if (reg < kRegisterCount) {
            quint8 value = 0;
            for (int bit = 0; bit < 8; ++bit) {
                Pin *p = (port == 0) ? pin(bit) : pin(8 + bit);
                const LogicValue lv = (p && p->node()) ? p->node()->resolvedValue() : LogicValue::Undefined;
                if (lv == LogicValue::High)
                    value |= static_cast<quint8>(1u << bit);
            }
            m_registers[reg] = value;
        }
        m_pc += 3;
        break;
    }

    case 0x10: { // MOV Rreg, #imm8
        const quint8 reg = fetch(1);
        const quint8 imm = fetch(2);
        if (reg < kRegisterCount) m_registers[reg] = imm;
        m_pc += 3;
        break;
    }

    case 0x11: { // MOV Rdst, Rsrc
        const quint8 dst = fetch(1);
        const quint8 src = fetch(2);
        if (dst < kRegisterCount && src < kRegisterCount) m_registers[dst] = m_registers[src];
        m_pc += 3;
        break;
    }

    case 0x20: { // ADD Rdst, Rsrc
        const quint8 dst = fetch(1);
        const quint8 src = fetch(2);
        if (dst < kRegisterCount && src < kRegisterCount)
            m_registers[dst] = static_cast<quint8>(m_registers[dst] + m_registers[src]);
        m_pc += 3;
        break;
    }

    case 0x21: { // ADD Rreg, #imm8
        const quint8 reg = fetch(1);
        const quint8 imm = fetch(2);
        if (reg < kRegisterCount)
            m_registers[reg] = static_cast<quint8>(m_registers[reg] + imm);
        m_pc += 3;
        break;
    }

    case 0x30: { // JMP addr16
        const quint16 addrHi = fetch(1);
        const quint16 addrLo = fetch(2);
        m_pc = static_cast<quint16>((addrHi << 8) | addrLo);
        break; // توجه: m_pc += 3 عمداً اینجا نیست، چون خود JMP مقدار PC را مستقیم تنظیم می‌کند
    }

    case 0x31: { // JNZ Rreg, addr16
        const quint8 reg = fetch(1);
        const quint16 addrHi = fetch(2);
        const quint16 addrLo = fetch(3);
        const bool takeJump = (reg < kRegisterCount) && (m_registers[reg] != 0);
        if (takeJump)
            m_pc = static_cast<quint16>((addrHi << 8) | addrLo);
        else
            m_pc += 4;
        break;
    }

    case 0x32: { // JZ Rreg, addr16
        const quint8 reg = fetch(1);
        const quint16 addrHi = fetch(2);
        const quint16 addrLo = fetch(3);
        const bool takeJump = (reg < kRegisterCount) && (m_registers[reg] == 0);
        if (takeJump)
            m_pc = static_cast<quint16>((addrHi << 8) | addrLo);
        else
            m_pc += 4;
        break;
    }

    case 0xFF: // HALT
        m_halted = true;
        break;

    default:
        // دستور ناشناخته - برای جلوگیری از رفتار تعریف‌نشده، متوقف می‌شویم
        m_halted = true;
        break;
    }
}

void MCU::updatePortPins()
{
    for (int bit = 0; bit < 8; ++bit) {
        if (Pin *p = pin(bit)) {
            const bool isOutput = (m_portP0Dir & (1u << bit)) != 0;
            p->setDirection(isOutput ? PinDirection::Output : PinDirection::Input);
            if (isOutput)
                p->setDrivenValue((m_portP0 & (1u << bit)) ? LogicValue::High : LogicValue::Low);
            else
                p->setDrivenValue(LogicValue::Undefined); // این پایه دیگر گره خودش را نمی‌راند
        }
        if (Pin *p = pin(8 + bit)) {
            const bool isOutput = (m_portP1Dir & (1u << bit)) != 0;
            p->setDirection(isOutput ? PinDirection::Output : PinDirection::Input);
            if (isOutput)
                p->setDrivenValue((m_portP1 & (1u << bit)) ? LogicValue::High : LogicValue::Low);
            else
                p->setDrivenValue(LogicValue::Undefined);
        }
    }
}

void MCU::resetSimulation()
{
    m_pc = 0;
    m_halted = false;
    m_registers.fill(0);
    m_ram.fill(0);
    m_portP0 = 0;
    m_portP1 = 0;
    m_portP0Dir = 0xFF; // پیش‌فرض: همه پایه‌ها Output (سازگار با فازهای قبلی)
    m_portP1Dir = 0xFF;
    updatePortPins();
}
