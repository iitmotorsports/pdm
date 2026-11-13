#! /usr/bin/env bash
openocd -f interface/stlink.cfg -f target/stm32g4x.cfg \
        -c "program build/Debug/pdm.elf verify reset exit"
