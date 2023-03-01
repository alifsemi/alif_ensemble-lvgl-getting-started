# ensemble-lvgl-getting-started_DEV
Demonstrations of the LVGL graphics library on Alif Ensemble Development Kit.

Requirements:

1. LVLG v1.08
2. Ensemble E7 devkit Rev-C
3. E43RB, E43GB or E50RA LCD panels
4. Edit RTE_Device.h, line #412
    For E43GB LCD,
    #define RTE_ILI9806E_PANEL_VARIANT  1
    For E50RA LCD,
    #define RTE_ILI9806E_PANEL_VARIANT  2
