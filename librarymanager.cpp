#include "librarymanager.h"
#include "passives.h"
#include "logicgates.h"
#include "semiconductors.h"
#include "sources.h"
#include "interactivecomponents.h"
#include "measurementtools.h"
#include "mcu.h"
#include "lcd.h"
#include "keypad.h"
#include "adc.h"
#include "dac.h"
#include "externalmemory.h"




QList<ComponentInfo> LibraryManager::m_components;

void LibraryManager::initialize()
{
    if(!m_components.isEmpty())
        return;

    // --- ثبت مقاومت (کد قبلی شما بدون تغییر) ---
    ComponentInfo resistor;
    resistor.name = "Resistor";
    resistor.category = "Passive";
    resistor.description = "Electrical Resistor";
    resistor.creator = []()-> Component*
    {
        return new Resistor();
    };





    m_components.append(resistor);

    // --- ۲. ثبت خازن (این بخش جدید را اضافه کنید) ---
    ComponentInfo capacitor;
    capacitor.name = "Capacitor";
    capacitor.category = "Passive"; // چون خازن هم یک قطعه پسیو یا غیرفعال است
    capacitor.description = "Electrical Capacitor";

   capacitor.creator = []() -> Component*
    {
        return new Capacitor();
    };

    m_components.append(capacitor);

/*    capacitor.creator = []()
    {
        return new Capacitor; // این تابع وقتی کاربر روی خازن کلیک کند، یک خازن جدید می‌سازد
    };*/


    // ثبت دیود
    ComponentInfo diode;
    diode.name = "Diode";
    diode.category = "Discrete"; // دسته‌بندی قطعات مجزا یا نیمه‌هادی
    diode.description = "Semiconductor Diode";
    diode.creator = []()

    -> Component*
    {
            return new Diode();

    };
    m_components.append(diode);

    // ثبت منبع ولتاژ DC
    ComponentInfo dcSource;
    dcSource.name = "DC Source";
    dcSource.category = "Sources"; // دسته‌بندی منابع تغذیه
    dcSource.description = "Direct Current Voltage Source";
    dcSource.creator = []() { return new DCVoltage; };
    m_components.append(dcSource);

    // ثبت باتری (فاز ۱۲)
    ComponentInfo batteryInfo;
    batteryInfo.name = "Battery";
    batteryInfo.category = "Sources";
    batteryInfo.description = "Battery (ideal voltage source in this phase)";
    batteryInfo.creator = []() { return new Battery; };
    m_components.append(batteryInfo);


    // ثبت سلف
    ComponentInfo inductor;
    inductor.name = "Inductor"; inductor.category = "Passive"; inductor.description = "Inductor";
    inductor.creator = []() { return new Inductor; };
    m_components.append(inductor);

    // ثبت زمین
    ComponentInfo ground;
    ground.name = "Ground"; ground.category = "Sources"; ground.description = "Ground Reference";
    ground.creator = []() { return new Ground; };
    m_components.append(ground);

    // ثبت ترانزیستور
    ComponentInfo transistor;
    transistor.name = "Transistor NPN"; transistor.category = "Semiconductors"; transistor.description = "NPN Transistor";
    transistor.creator = []() { return new TransistorNPN; };
    m_components.append(transistor);

    // ثبت گیت منطقی
    ComponentInfo gateAnd;
    gateAnd.name = "Gate AND"; gateAnd.category = "Logic Gates"; gateAnd.description = "AND Logic Gate";
    gateAnd.creator = []() { return new GateAND; };
    m_components.append(gateAnd);

    ComponentInfo gateOr;
    gateOr.name = "Gate OR"; gateOr.category = "Logic Gates"; gateOr.description = "OR Logic Gate";
    gateOr.creator = []() { return new GateOR; };
    m_components.append(gateOr);

    ComponentInfo gateNot;
    gateNot.name = "Gate NOT"; gateNot.category = "Logic Gates"; gateNot.description = "NOT Logic Gate";
    gateNot.creator = []() { return new GateNOT; };
    m_components.append(gateNot);

    ComponentInfo gateNand;
    gateNand.name = "Gate NAND"; gateNand.category = "Logic Gates"; gateNand.description = "NAND Logic Gate";
    gateNand.creator = []() { return new GateNAND; };
    m_components.append(gateNand);

    ComponentInfo gateXor;
    gateXor.name = "Gate XOR"; gateXor.category = "Logic Gates"; gateXor.description = "XOR Logic Gate";
    gateXor.creator = []() { return new GateXOR; };
    m_components.append(gateXor);

    ComponentInfo dFlipFlop;
    dFlipFlop.name = "D Flip-Flop"; dFlipFlop.category = "Logic Gates"; dFlipFlop.description = "Edge-Triggered D Flip-Flop";
    dFlipFlop.creator = []() { return new DFlipFlop; };
    m_components.append(dFlipFlop);

    // --- قطعات تعاملی ---
    ComponentInfo switchInfo;
    switchInfo.name = "Switch"; switchInfo.category = "Interactive"; switchInfo.description = "Digital ON/OFF Switch";
    switchInfo.creator = []() { return new Switch; };
    m_components.append(switchInfo);

    ComponentInfo pushButtonInfo;
    pushButtonInfo.name = "Push Button"; pushButtonInfo.category = "Interactive"; pushButtonInfo.description = "Momentary Push Button";
    pushButtonInfo.creator = []() { return new PushButton; };
    m_components.append(pushButtonInfo);

    ComponentInfo ledInfo;
    ledInfo.name = "LED"; ledInfo.category = "Interactive"; ledInfo.description = "Light Emitting Diode Indicator";
    ledInfo.creator = []() { return new LED; };
    m_components.append(ledInfo);

    // --- مولد کلاک ---
    ComponentInfo clockInfo;
    clockInfo.name = "Clock Generator"; clockInfo.category = "Sources"; clockInfo.description = "Oscillating Digital Clock Source";
    clockInfo.creator = []() { return new ClockGenerator; };
    m_components.append(clockInfo);

    // --- ابزار اندازه‌گیری ---
    ComponentInfo logicProbeInfo;
    logicProbeInfo.name = "Logic Probe"; logicProbeInfo.category = "Measurement"; logicProbeInfo.description = "Displays H/L/Undefined at a node";
    logicProbeInfo.creator = []() { return new LogicProbe; };
    m_components.append(logicProbeInfo);

    ComponentInfo voltmeterInfo;
    voltmeterInfo.name = "Digital Voltmeter"; voltmeterInfo.category = "Measurement"; voltmeterInfo.description = "Displays mapped voltage at a node";
    voltmeterInfo.creator = []() { return new DigitalVoltmeter; };
    m_components.append(voltmeterInfo);

    ComponentInfo oscilloscopeInfo;
    oscilloscopeInfo.name = "Oscilloscope"; oscilloscopeInfo.category = "Measurement"; oscilloscopeInfo.description = "Two-channel logic waveform display";
    oscilloscopeInfo.creator = []() { return new Oscilloscope; };
    m_components.append(oscilloscopeInfo);

    ComponentInfo analogVoltmeterInfo;
    analogVoltmeterInfo.name = "Analog Voltmeter"; analogVoltmeterInfo.category = "Measurement"; analogVoltmeterInfo.description = "Real DC voltage across two points (needs AnalogSolver)";
    analogVoltmeterInfo.creator = []() { return new AnalogVoltmeter; };
    m_components.append(analogVoltmeterInfo);

    ComponentInfo analogAmmeterInfo;
    analogAmmeterInfo.name = "Analog Ammeter"; analogAmmeterInfo.category = "Measurement"; analogAmmeterInfo.description = "Real DC current through a series branch (needs AnalogSolver)";
    analogAmmeterInfo.creator = []() { return new AnalogAmmeter; };
    m_components.append(analogAmmeterInfo);

    // --- میکروکنترلر ---
    ComponentInfo mcuInfo;
    mcuInfo.name = "MCU"; mcuInfo.category = "Advanced"; mcuInfo.description = "Simple educational microcontroller (PC/Registers/RAM/I-O, HEX loadable)";
    mcuInfo.creator = []() { return new MCU; };
    m_components.append(mcuInfo);

    // --- نمایشگر کاراکتری و صفحه‌کلید (بخش ۷.۸ و ۷.۹ مستند) ---
    ComponentInfo lcdInfo;
    lcdInfo.name = "LCD 16x2"; lcdInfo.category = "Advanced"; lcdInfo.description = "16x2 Character LCD (8-bit data bus + RS + EN)";
    lcdInfo.creator = []() { return new LCD; };
    m_components.append(lcdInfo);

    ComponentInfo keypadInfo;
    keypadInfo.name = "Keypad 4x4"; keypadInfo.category = "Advanced"; keypadInfo.description = "4x4 Matrix Keypad (row/column scan simulation)";
    keypadInfo.creator = []() { return new Keypad; };
    m_components.append(keypadInfo);

    ComponentInfo adcInfo;
    adcInfo.name = "ADC"; adcInfo.category = "Advanced"; adcInfo.description = "8-bit Analog-to-Digital Converter (needs AnalogSolver)";
    adcInfo.creator = []() { return new ADC; };
    m_components.append(adcInfo);

    ComponentInfo dacInfo;
    dacInfo.name = "DAC"; dacInfo.category = "Advanced"; dacInfo.description = "8-bit Digital-to-Analog Converter (needs AnalogSolver)";
    dacInfo.creator = []() { return new DAC; };
    m_components.append(dacInfo);

    ComponentInfo memInfo;
    memInfo.name = "EEPROM/RAM"; memInfo.category = "Advanced"; memInfo.description = "256-byte external memory chip (address/data bus + WE)";
    memInfo.creator = []() { return new ExternalMemory; };
    m_components.append(memInfo);
}

QList<ComponentInfo> LibraryManager::components()
{
    return m_components;
}