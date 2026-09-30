# NXP Application Code Hub
[<img src="https://mcuxpresso.nxp.com/static/icon/nxp-logo-color.svg" width="100"/>](https://www.nxp.com)

## BCTU Control of ADC Sampling
This demo illustrates the use of the SAR ADC and BCTU modules, where ADC conversions are initiated through a BCTU software trigger. The implementation relies on RTD high-level (MCAL) drivers to perform ADC channel conversions.

The BCTU operates in Control Mode, taking full control of the ADC module and managing all ADC sampling through software-triggered conversions. Instead of sampling external analog inputs, the ADC channels are configured to measure the internal bandgap reference and VREFH voltage, typically around 1.2 V and 5 V, respectively.

[<p align="center"><img src="images/FRDM_A_S32K344_ADC_BCTU_DMA_Graph.png" width="500"/></p>](./images/FRDM_A_S32K344_ADC_BCTU_DMA_Graph.png)

#### Boards: FRDM-A-S32K344
#### Categories: Motor Control
#### Peripherals: ADC, BCTU
#### Toolchains: S32 Design Studio IDE

## Table of Contents
1. [Software and Tools](#step1)
2. [Hardware](#step2)
3. [Setup](#step3)
4. [Results](#step4)
5. [Support](#step5)
6. [Release Notes](#step6)

## 1. Software and Tools<a name="step1"></a>
This example was developed using the FRDM Automotive Bundle for S32K3 + S32M27. To download and install the complete software and tools ecosystem, use the following link:
- [ FRDM Automotive S32K3 + S32M27 Board Installation Package](https://www.nxp.com/app-autopackagemgr/automotive-software-package-manager:AUTO-SW-PACKAGE-MANAGER?currentTab=0&selectedDevices=S32K3&applicationVersionID=203)

## 2. Hardware<a name="step2"></a>
- Type-C USB cable
- Personal Computer
- [FRDM-A-S32K344](https://www.nxp.com/design/design-center/development-boards-and-designs/FRDM-A-S32K344)<p><img src="https://www.nxp.com/assets/images/en/dev-board-image/FRDM-A-S32K344-TOP.png" width="500"></p>

## 3. Setup<a name="step3"></a>
### 3.1 Import the project to S32 Design Studio IDE

1. Open S32 Design Studio IDE, in the Dashboard Panel, choose **Import project from Application Code Hub**.
[<p align="center"><img src="images/import_project_1.png" width="300"/></p>](./images/import_project_1.png)

2. You can find the demo you need by searching for the name directly "BCTU Control of ADC Sampling". Open the project, click the **GitHub link** from this window, S32 Design Studio IDE will automatically retrieve project attributes then click **Next>**.
[<p align="center"><img src="images/import_project_3.png" width="500"/></p>](./images/import_project_2.png)

3. Select **main** branch and then click **Next>**.

4. Select your local path for the repo in **Destination->Directory** window. The S32 Design Studio IDE will clone the repo into this path, click **Next>**.

5. Select **Import existing Eclipse projects** then click **Next>**.

6. Select the project in this repo (only one project in this repo) then click **Finish**.

### 3.2 Generating, building and running the example application
1. In Project Explorer, right-click the project and select **Update Code and Build Project**. This will generate the configuration (Pins, Clocks, Peripherals), update the source code and build the project using the active configuration (e.g. Debug_FLASH).
Make sure the build completes successfully and the *.elf file is generated without errors.
[<p align="center"><img src="images/update_and_build.png" width="300"/></p>](./images/update_and_build.png)
Press **Yes** in the **SDK Component Management** pop-up window to continue.

2. Go to **Debug** and select **Debug Configurations**. There will be a debug configuration for this project:
[<p align="center"><img src="images/Debug_config.png" width="300"/></p>](./images/Debug_config.png)

        Configuration Name                  Description
        -------------------------------     -----------------------
        $(example)_debug_flash_pemicro      Debug the FLASH configuration using PEmicro probe

    Select the desired debug configuration and click on **Debug**. Now the perspective will change to the **Debug Perspective**.
    Use the controls to control the program flow.

## 4. Results<a name="step4"></a>

The demo is divided into 3 parts:

   Part 1: BCTU control mode, using list mode for conversion on all ADC instances in series, data conversion is updated by DMA.

   Part 2: BCTU control mode for parallel conversions from all ADC instances using list mode, data conversion is updated by DMA.

   Part 3: Verify that the data transmitted to the larger buffers through DMA is correct.
       [<p align="center"><img src="images/ADC_BCTU_SWTrigger.png" width="500"/></p>](./images/ADC_BCTU_SWTrigger.png)

When you suspend the debugging session, in *Expressions tab* you can observe the results: **BctuDmaFifo1** and **BctuDmaFifo2**, which are used to store raw data from BCTU FIFOs according to the trigger and BCTU List, then all of them are transferred to ***DmaCh1_DestinationBuffer*** and ***DmaCh17_DestinationBuffer*** which will store all the corresponding measurements for each BCTU FIFO, after that a check is performed to see if the results transferred to both buffers are correct.

Verification criteria for a successful example run is **bStatus** being TRUE at the end of the example.
An example check file was included that is used to automatically test examples, this writes a pass or fail status in the memory at a certain address.
    [<p align="center"><img src="images/FIFOs_Results.png" width="400"/></p>](./images/FIFOs_Results.png)

## 5. Support<a name="step5"></a>
For general technical questions related to NXP microcontrollers, please use the [NXP Community Forum](https://community.nxp.com/).

#### Project Metadata

<!----- Boards ----->
[![Board badge](https://img.shields.io/badge/Board-FRDM&ndash;A&ndash;S32K344-blue)](https://mcuxpresso.nxp.com/appcodehub?hwBoard=FRDM-A-S32K344)

<!----- Categories ----->
[![Category badge](https://img.shields.io/badge/Category-MOTOR%20CONTROL-yellowgreen)](https://mcuxpresso.nxp.com/appcodehub?category=motor_control)
[![Category badge](https://img.shields.io/badge/Category-ANALOG%20FRONT%20END-yellowgreen)](https://mcuxpresso.nxp.com/appcodehub?category=analog_front_end)

<!----- Peripherals ----->
[![Peripheral badge](https://img.shields.io/badge/Peripheral-BCTU-yellow)](https://mcuxpresso.nxp.com/appcodehub?peripheral=bctu)
[![Peripheral badge](https://img.shields.io/badge/Peripheral-ADC-yellow)](https://mcuxpresso.nxp.com/appcodehub?peripheral=adc)
[![Peripheral badge](https://img.shields.io/badge/Peripheral-DMA-yellow)](https://mcuxpresso.nxp.com/appcodehub?peripheral=dma)

<!----- Toolchains ----->
[![Toolchain badge](https://img.shields.io/badge/Toolchain-S32%20DESIGN%20STUDIO%20IDE-orange)](https://mcuxpresso.nxp.com/appcodehub?toolchain=s32_design_studio_ide)

Questions regarding the content/correctness of this example can be entered as Issues within this GitHub repository.

>**Note**: For more general technical questions regarding NXP Microcontrollers and the difference in expected functionality, enter your questions on the [NXP Community Forum](https://community.nxp.com/)

[![Follow us on Youtube](https://img.shields.io/badge/Youtube-Follow%20us%20on%20Youtube-red.svg)](https://www.youtube.com/NXP_Semiconductors)
[![Follow us on LinkedIn](https://img.shields.io/badge/LinkedIn-Follow%20us%20on%20LinkedIn-blue.svg)](https://www.linkedin.com/company/nxp-semiconductors)
[![Follow us on Facebook](https://img.shields.io/badge/Facebook-Follow%20us%20on%20Facebook-blue.svg)](https://www.facebook.com/nxpsemi/)
[![Follow us on Twitter](https://img.shields.io/badge/X-Follow%20us%20on%20X-black.svg)](https://x.com/NXP)

## 6. Release Notes<a name="step6"></a>
| Version | Description / Update                           | Date                           |
|:-------:|------------------------------------------------|-------------------------------:|
| 1.0     | Initial release on Application Code Hub        | September 30<sup>th</sup> 2026 |