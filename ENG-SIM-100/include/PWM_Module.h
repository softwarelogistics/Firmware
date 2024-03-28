#ifndef PWM_Module_h
#define PWM_Module_h

#include "driver/ledc.h"
#include "Console.h"

class PWM_Module {
    private:
        Console *m_pConsole;
        uint64_t m_nextPrint = 0;

    public:
        PWM_Module(Console *console) {
            m_pConsole = console;
        }

        void Setup();
        void Stop();
        void Loop();
    private:
};

#endif