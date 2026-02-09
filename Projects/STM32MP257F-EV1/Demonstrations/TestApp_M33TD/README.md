# TestApp_M33TD Demonstration Application

---

## Application Description

The TestApp_M33TD demonstration highlights the capabilities of the STM32MP2 platform for the M33TDCID profile, showcasing the Cortex‑M33 as the primary CPU and the Cortex‑A35 as a high-performance coprocessor.

This project uses the “Utility-integrated” design and enables the CM33‑NS **Non‑Secure Application Manager** bootstrap based on the shared utility stack `Utilities/M33TD_NSAppCore` (Logger, RemoteProc, SCMI Manager, Watchdog Monitor, OpenAMP, etc.). Board-specific drivers and adaptations for this demonstration are implemented under `CM33/NonSecure/FREERTOS/M33TD_NSAppCore/AppDriver/`.

TestApp then demonstrates how to build a custom application on top of this common stack by adding a dedicated **TestApp Task**, which runs a sequence of HAL-based functional tests to validate platform configuration and example peripherals in the non-secure (NS) environment.

---

## Key Features

- **FreeRTOS Multitasking**: Runs multiple tasks in the non-secure environment.
- **Integrate Utility Stack**: Uses `Utilities/M33TD_NSAppCore` for reusable tasks and thin driver abstractions.
- **A35 Coprocessor Management**: Uses the RemoteProc task to manage the A35 core lifecycle via TF‑M secure services.
- **UserApp Task**: LED activity task to validate system liveness.
- **TestApp Task**: Runs a suite of tests sequentially (automatic/manual behavior controlled by build options).
- **Optional Display Pipeline**: Same build-time display option support as the shared stack (see `BUILD_CONFIG` / `BOOT_SPLASHSCREEN`).

---

## Purpose

This is a FreeRTOS-based multitask application running in the NS processing environment, while the TF‑M secure application operates in the secure processing environment. The secure application acts as a client to process secure services requested by both the A35 and CM33‑NS.

TestApp implements the Non‑Secure Application Manager around the `Utilities/M33TD_NSAppCore` stack. The `NSCoreApp_Init()` bootstrap configures the stack and starts the enabled tasks below:

1. **NSCoreApp (Bootstrap)**:
   - Initializes the common stack and starts the enabled tasks.

2. **Logger Task**:
   - Centralized logging, with optional real-time output.

3. **UserApp Task**:
   - Example application task (LED activity) to validate system liveness.

4. **RemoteProc Task**:
   - Manages the A35 coprocessor lifecycle via TF‑M secure services:
     - Optional auto-start at boot (`REMOTE_PROC_AUTO_START`).
     - CPU status retrieval.
     - Crash detection and recovery (stop/start sequence).

5. **SCMI Manager Task**:
   - Handles SCMI notifications and related TF‑M forwarding.

6. **Watchdog Monitor Task**:
   - Supervises system health and watchdog-related handling.

7. **OpenAMP Task**:
   - Provides the RPMsg communication layer used by the stack to interact with the A35/Linux side.

8. **Button Monitor Task**:
   - Monitors the USER button and can trigger application-defined actions (implementation may vary by build configuration).

9. **Display Task (optional)**:
   - Available when the display pipeline is enabled in the build (see `BUILD_CONFIG=FULL`).

10. **TestApp Task**:
   - Executes a sequence of tests to validate the M33 example functionality under the M33TDCID profile.

---

## Prerequisite Hardware & Software Environment Setup

- **Trusted Firmware-M**: The source code must be installed under the `Middlewares/Third_Party` directory with the path `Middlewares/Third_Party/trusted-firmware-m`. Ensure the correct version is used as described in the STM32 MPU release note.
- **Supported Devices**: This example runs on STM32MP25xx devices and has been tested with the STMicroelectronics STM32MP257F-EV1 board. It can be tailored to other supported devices and development boards.
- **ST-Link Connection**: Connect the ST-Link cable to the PC USB port to display traces.

### Software Versions
- **Trusted Firmware-M (TFM)**: Refer to the [Trusted Firmware-M wiki](https://wiki.st.com/stm32mpu/Category:Trusted_Firmware-M) for recommended versions and integration details.
- **External Device Tree (externalDT)**: See the [External Device Tree wiki](https://wiki.st.com/stm32mpu/External_device_tree) for guidance on obtaining and using external DT sources.
- **STM32CubeIDE**: For supported IDE versions and ecosystem information, refer the [STM32 MPU wiki](https://wiki.st.com/stm32mpu/).

---

## Artifact flow (build → sign → deploy)

This is a high-level view; the later sections in this README give the board-specific filenames and flashing steps.

<pre>
┌──────────────┐
│ TF-M build   │
└─┬────────────┘
  +-> bl2*.stm32
  +-> ddr_phy*_Signed.bin
  +-> tfm_s.bin

┌───────────────────┐
│ CM33-NS app build │
└─┬─────────────────┘
  +-> ${PROJECT_NAME}.bin

┌─────────────────────────────┐
│ postbuild (assemble + sign) │
└─┬───────────────────────────┘
  | inputs: ${PROJECT_NAME}.bin + tfm_s.bin
   +-> tfm-testapp-*_s_ns_Signed.bin

Deploy into OSTL image tree
   bl2*.stm32            -> .../images/stm32mp2-m33td/arm-trusted-firmware-m/bl2
   ddr_phy*_Signed.bin   -> .../images/stm32mp2-m33td/m33-firmware
   tfm-testapp-*_s_ns_Signed.bin -> .../images/stm32mp2-m33td/m33-firmware
</pre>

---

## To Build CMake Project for M33TDCID (Through Command Line)

### Compilation Instructions
- Ensure an ARM cross-compiler toolchain with `arm-none-eabi-gcc` is available in `PATH`.
- When using `-G"Unix Makefiles"` on Windows, also ensure `make` is available in `PATH`.
- Example for STM32CubeIDE 1.19.0.25A9 (Git Bash):
   ```bash
   export PATH="/c/ST/STM32CubeIDE_1.19.0.25A9/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.13.3.rel1.win32_1.0.0.202411081344/tools/bin:/c/ST/STM32CubeIDE_1.19.0.25A9/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.make.win32_2.2.0.202409170845/tools/bin:$PATH"
   ```

---

## Build Procedure

### Secure Build (TFM)
1. Navigate to `Firmware/Middlewares/Third_Party/trusted-firmware-m`.
2. For SD card development mode, execute the following command:
   ```bash
   cmake -B config_default -G"Unix Makefiles" -DTFM_PLATFORM=stm/stm32mp257f_ev1 -DTFM_TOOLCHAIN_FILE=toolchain_GNUARM.cmake -DSTM32_BOOT_DEV=sdmmc1 -DTFM_PROFILE=profile_medium -DSTM32_M33TDCID=ON -DCMAKE_BUILD_TYPE=Relwithdebinfo -DNS=OFF -DDTS_EXT_DIR=<EXT_DT_DIR> -DDTS_BOARD_BL2=stm32mp2/m33-td/mcuboot/stm32mp257f-ev1-cm33tdcid-ostl-sdcard-bl2.dts -DDTS_BOARD_S=stm32mp2/m33-td/tfm/stm32mp257f-ev1-cm33tdcid-ostl-sdcard-s.dts \
   -DDTS_BOARD_NS=stm32mp2/m33-td/tfm/stm32mp257f-ev1-cm33tdcid-ostl-ns.dts
   ```
3. Build the project:
   ```bash
   cmake --build config_default -- install
   ```

**Note**: The external DT repository is placed in the `Utilities` directory.

---

### Non-Secure M33 Firmware Build (TestApp_M33TD Application)
1. Navigate to `Firmware/Projects/STM32MP257F-EV1/Demonstrations/TestApp_M33TD`.
2. Run the following command:
   ```bash
   cmake -G"Unix Makefiles" -B build \
     -DTFM_BUILD_DIR=<TFM_BUILD_DIRECTORY> \
     -DENABLE_AUTO_TEST=ON
   ```

   - If `TFM_BUILD_DIR` is not specified, the default path `Firmware/Middlewares/Third_Party/trusted-firmware-m/config_default` will be used (assuming the TF‑M secure build step is completed).
   - Common build options (see `CMakeLists.txt`):
     - `-DBUILD_CONFIG=FULL|MINIMUM` (default: `FULL`)
       - `FULL`: enables display pipeline (adds display panel files and HAL LTDC/LVDS dependencies)
       - `MINIMUM`: disables display pipeline
     - `-DBOOT_SPLASHSCREEN=DYNAMIC|STATIC` (default: `DYNAMIC`, only meaningful when `BUILD_CONFIG=FULL`)
     - `-DREMOTE_PROC_AUTO_START=ON|OFF` (default: `ON`)
       - `-DREALTIME_DEBUG_LOG_ENABLED=ON|OFF` (default: `ON`)
     - `-DFAULT_EXCEPTION_ENABLE=ON|OFF` (default: `ON`)
     - `-DFAULT_EXCEPTION_BACKTRACE_ENABLE=ON|OFF` (default: `ON`)
       - `-DENABLE_AUTO_TEST=ON|OFF` (default: `ON`)

    **Note**: These CMake options are converted into numeric compile-time macros (`...=1` or `...=0`) by the project CMake logic.

3. Build the project:
   ```bash
   make -C build all
   ```

---

## To Build Native STM32CubeIDE Project

### Project Structure
```
TestApp_M33TD
├── TestApp_M33TD_CM33
│   ├── TestApp_M33TD_CM33_NonSecure (Non-Secure STM32CubeIDE M33 project)
│   └── TestApp_M33TD_CM33_trusted-firmware-m (Secure CMake project)
```

**Note**: Refer to [this wiki](https://wiki.st.com/stm32mpu/How_to_create_an_M33-TD_boot_project_using_STM32CubeIDE#) for instructions on importing a CMake project.

### Build Procedure

#### Secure TFM Firmware Build
1. Configure the CMake build options:
   - Navigate to `TestApp_M33TD_CM33_trusted-firmware-m` and update the CMake settings:
     ```
     -DDEBUG_AUTHENTICATION=FULL  # Enabled for Debug Purpose, Default: this option is removed
     -DTFM_PLATFORM=stm/stm32mp257f_ev1
     -DTFM_TOOLCHAIN_FILE=toolchain_GNUARM.cmake
     
     -DSTM32_BOOT_DEV=sdmmc1  # Building TFM for "sdcard_sdcard" bootdevice mode
     -DTFM_PROFILE=profile_medium
     -DSTM32_M33TDCID=ON
     -DCMAKE_BUILD_TYPE=Relwithdebinfo
     -DNS=OFF
     -DDTS_EXT_DIR=../../../../../../../../../Firmware/Utilities/dt-stm32mp
     -DDTS_BOARD_BL2=stm32mp2/m33-td/mcuboot/stm32mp257f-ev1-cm33tdcid-ostl-sdcard-bl2.dts  # External DT file for sdcard_sdcard bootdevice mode
     -DDTS_BOARD_S=stm32mp2/m33-td/tfm/stm32mp257f-ev1-cm33tdcid-ostl-sdcard-s.dts          # External DT file for sdcard_sdcard bootdevice mode
     -DDTS_BOARD_NS=stm32mp2/m33-td/tfm/stm32mp257f-ev1-cm33tdcid-ostl-ns.dts               # External DT file common for all bootdevice modes
     ```
2. Configure the TFM CMake project:
   - Right-click on `TestApp_M33TD_CM33_trusted-firmware-m` -> `CMake Configure`.
3. Build the TFM CMake project:
   - Right-click on `TestApp_M33TD_CM33_trusted-firmware-m` -> `Build Project`.

#### Non-Secure STM32CubeIDE Project Build
1. Build the project:
   - Select the build configuration as per the TFM version:
     - Choose `CM33TDCID_m33_ns_tfm_s_sign`.
   - Right-click on `TestApp_M33TD_CM33_NonSecure` -> `Build Project`.

---

## Output

- The generated binaries will be located in the `bin` folder:
```
cd bin/
```
   - `StarterApp_M33TD_CM33_NonSecure.bin`
   - `tfm_s_ns_signed.bin`
   - `ddr_phy_signed.bin`
   - `bl2.stm32`

   **Note**: For other boot device modes, TF‑M secure build commands need to be adjusted, and the generated binary should be renamed accordingly. Refer to the [How to Generate Binaries for Different Boot Modes](#how-to-generate-binaries-for-different-boot-modes) section.

---

For detailed instructions on flashing the generated binaries to your device, see [How to Flash Binaries](#how-to-flash-binaries).

---

## TestApp Task Configuration

The **TestApp Task** is responsible for running a sequence of M33 example tests. Each test follows these steps:
1. Acquire the required resources to run the test.
2. Initialize the necessary peripherals.
3. Execute the test.
4. Deinitialize the test and release the acquired resources.

### Test Categories
Tests are categorized into two types based on their execution requirements:
- **AUTO**: Tests that can be executed without external dependencies.
- **MANUAL**: Tests that require external dependencies or user interaction.

### Controlling Test Execution
While building the `TestApp_M33TD`, the test execution behavior can be controlled by passing the following CMake options or CubeIDE preprocessor definitions:

#### **Scenario 1: ENABLE_AUTO_TEST is ON (Set to 1)**
- **Behavior**:
  - The TestApp will execute automatically.
  - **AUTO** tests will be fully executed, including all steps (AcquireResource → Init → Run → Deinit → ReleaseResource).
  - **MANUAL** tests will be partially executed, performing only the resource acquisition and release steps (AcquireResource → ReleaseResource).
- **Purpose**:
  - This scenario validates the correct runtime configuration settings required for specific M33 examples in the M33TDCID profile.

#### **Scenario 2: ENABLE_AUTO_TEST is OFF (Set to 0)**
- **Behavior**:
  - The TestApp will execute tests fully based on **User Button 2** actions.
  - Both **AUTO** and **MANUAL** tests will be executed completely (AcquireResource → Init → Run → Deinit → ReleaseResource).

### How to Run Manual Tests

#### I3C Test
- Ensure the target I2C board is ready before executing the I3C controller test.
- Refer to the examples provided at `Firmware/Projects/STM32MP257F-EV1/Examples/I3C` to set up the target I2C board.

#### SPI Test
- Ensure the slave SPI board is ready before executing the SPI Master test.
- Refer to the examples provided at `Firmware/Projects/STM32MP257F-EV1/Examples/SPI` to set up the slave board for testing.

#### I2C Test
- Ensure the I2C Low Power Display is connected to the board.
- Refer to the example at `Firmware/Projects/STM32MP257F-EV1/Demonstrations/M33TD_DEMO/DisplayDemo_M33TD` to set up the display.

---

### Example CMake Options
To control the test execution behavior, use the following CMake options during the build process:
```bash
-DENABLE_AUTO_TEST=ON   # Enables automatic execution of Test Sequence.
-DENABLE_AUTO_TEST=OFF  # Disables automatic execution; tests are triggered manually.
```

### Example CubeIDE Preprocessor Definitions
To control the test execution behavior in CubeIDE, define the following preprocessor macros in project settings under compiler preprocessor configuration:
```c
ENABLE_AUTO_TEST=1  // Enables automatic execution of Test Sequence.
ENABLE_AUTO_TEST=0  // Disables automatic execution; tests are triggered manually.
```

---

This section ensures that developers can configure and control the execution of the TestApp task based on their requirements, providing flexibility for both automated and manual testing scenarios.

---

#### UART Console Output

Below is a sample UART log output from the CM33 during the boot and runtime sequence:

```
[INF] init:pmic@33 STPMIC:20 V1.1

[INF] welcome to MCUboot: v2.1.3-stm32mp-r2-rc8-1-gd0c0251d4
[INF] cpu: STM32MP257FAI Rev.Y
[INF] board: stm32mp257f eval1
[INF] board ID: MB1936 Var1.0 Rev.D-01
[INF] dts: stm32mp257f-ev1-cm33tdcid-ostl-sdcard-bl2.dts
[INF] boot device: sdmmc1
[INF] mcu sysclk: 400000000
[INF] Loading gpt header

[INF] Starting bootloader
[WRN] This device was provisioned with dummy keys.

[WRN] This device is NOT SECURE

[INF] PSA Crypto init done, sig_type: EC-P256
[INF] Primary   slot: version=0.1.0+0
[INF] Secondary slot: version=0.1.0+0
[INF] Image 1 RAM loading to 0xe060000 is succeeded.
[INF] Image 1 loaded from the primary slot
[INF] BL2: image 1, enable DDR-FW
[INF] Primary   slot: version=2.1.0+0
[INF] Secondary slot: version=2.1.0+0
[INF] Image 0 RAM loading to 0x80000000 is succeeded.
[INF] Image 0 loaded from the primary slot
[INF] Bootloader chainload address offset: 0x104400
[INF] Jumping to the first image slot
[INF] init:pmic@33 STPMIC:20 V1.1
[INF] Enable Macronix quad support
[INF] welcome to TF-M: v2.1.3-stm32mp-r2-rc8-1-gd0c0251d4
[INF] board: stm32mp257f eval1
[INF] dts: stm32mp257f-ev1-cm33tdcid-ostl-sdcard-s.dts
Booting TF-M v2.1.3
[WAR] This device was provisioned with dummy keys.
[WAR] This device is NOT SECURE
[Sec Thread] Secure image initializing!
INFO: low power firmware "v1.0.0.rc1"
[    0.000000] SCP-firmware v2.13.0-stm32mp-r3-rc5
[    0.000000]
[    0.000000] [FWK] Module initialization complete!
[INF][PS] Encryption alg: 0x5500100
[INF][Crypto] Init HW accelerator...
[INF][Crypto] Init HW accelerator... complete.
[NS] [INF] Non-Secure system starting...
[NS] [INF] STM32Cube FW version: v1.2.0-rc0
[NS] [INF] [DisplayTask] Display is initialized and ready...
[NS] [INF] [WdgMonitor] watchdog timeout: 120000ms
[NS] [INF] [OpenAMP] waiting remote processor initialisation...
[NS] [INF] [RemoteProc] starting copro cpu@0... (pending)
[NS] [INF] [TestApp] Acquiring resources for test: TIMERS
[NS] [INF] [TestApp] Running test: TIMERS
[NS] [INF] [RemoteProc] copro cpu@0 started.
[NS] [INF] [TestApp] Test TIMERS: PASSED
[NS] [INF] [TestApp] Releasing resources for test: TIMERS
[NS] [INF] [TestApp] Acquiring resources for test: I2C
[NS] [INF] [TestApp] Skipping manual test in auto Test mode: I2C
[NS] [INF] [TestApp] Releasing resources for test: I2C
[NS] [INF] [TestApp] Acquiring resources for test: SPI
[NS] [INF] [TestApp] Skipping manual test in auto Test mode: SPI
[NS] [INF] [TestApp] Releasing resources for test: SPI
[NS] [INF] [TestApp] Acquiring resources for test: ADC
[NS] [INF] [TestApp] Running test: ADC
[NS] [INF] [TestApp] Test ADC: PASSED
[NS] [INF] [TestApp] Releasing resources for test: ADC
[NS] [INF] [TestApp] Acquiring resources for test: I3C
[NS] [INF] [TestApp] Skipping manual test in auto Test mode: I3C
[NS] [INF] [TestApp] Releasing resources for test: I3C
[NS] [INF] [TestApp] All tests completed.
[NS] [INF] [OpenAMP]  Ready for RPMsg communication
```

### Understanding TestApp Logs

This log demonstrates the CM33 boot sequence (MCUboot/TF-M), secure→non-secure handover, and the main runtime tasks from the shared utility stack, plus the TestApp test execution flow.

> **Note:** The presence of `Skipping manual test in auto Test mode: ...` indicates the project was built with `ENABLE_AUTO_TEST=1` (for example via `-DENABLE_AUTO_TEST=ON`).

The log also reflects the specific firmware and configuration used for the device boot:

- **STM32Cube FW version**: `STM32Cube FW version: ...`
- **MCUboot version**: `welcome to MCUboot: ...`
- **TF-M version**: `welcome to TF-M: ...`
- **External device tree (DTS)**: `dts: ...-bl2.dts` and `dts: ...-s.dts` (compare with the `-DDTS_BOARD_...` options passed to the TF-M secure build)

Key runtime milestones:

- Display enabled (when `BUILD_CONFIG=FULL`): `[DisplayTask] Display is initialized and ready...`
- Watchdog monitor: `[WdgMonitor] watchdog timeout: ...`
- A35 bring-up: `[RemoteProc] starting ...` then `[RemoteProc] copro cpu@0 started.`
- TestApp progress:
  - Test start/finish lines (for example: `Running test: ...`, `Test ...: PASSED`, `All tests completed.`)
  - Auto-test behavior for MANUAL tests (for example: `Skipping manual test in auto Test mode: ...`)
- RPMsg ready: `[OpenAMP]  Ready for RPMsg communication`

To verify that the correct configuration is reflected, compare the DTS filename(s) and firmware versions shown in the log with the TF-M secure-build CMake options and external DTS files specified during the secure build (see [How to Generate Binaries for Different Boot Modes](#how-to-generate-binaries-for-different-boot-modes)).

### How to Test and Verify System Behavior

Once the system has completed boot (for example, you see `[RemoteProc] copro cpu@0 started.` and `[OpenAMP]  Ready for RPMsg communication`), you can run the checks below. For each test, look for the corresponding activity on the **CM33 UART console**.

1. **Verify TestApp automatic mode (`ENABLE_AUTO_TEST=1`)**
   - Build with `-DENABLE_AUTO_TEST=ON`.
   - Expected CM33 logs:
      - `[TestApp] ... Running test: ...` for **AUTO** tests
      - `[TestApp] Skipping manual test in auto Test mode: ...` for **MANUAL** tests
      - `[TestApp] All tests completed.`

2. **Verify TestApp manual mode (`ENABLE_AUTO_TEST=0`, USER2-triggered)**
   - Build with `-DENABLE_AUTO_TEST=OFF`.
   - Press **USER2** to trigger the TestApp sequence as described in [TestApp Task Configuration](#testapp-task-configuration).
   - Expected CM33 logs: both **AUTO** and **MANUAL** tests execute fully (no "Skipping manual test" messages).

3. **Button-triggered A35 reboot (USER2, very long press)**
   - Press **USER2** for **at least 5 seconds**.
   - When the OpenAMP power endpoint is enabled in the common stack, the Button Monitor task can trigger an M33-initiated reboot request to the remote processor.
   - Expected CM33 logs (may vary by build options):
      - `[OpenAMP] request remote processor reboot`
      - RemoteProc messages reflecting stop/start or recovery.

4. **Simulate a Linux crash and observe recovery**
   - On the Linux console (as `root`), run:
      ```bash
      sync; sleep 2; sync; echo c > /proc/sysrq-trigger
      ```
   - On the CM33 UART, RemoteProc should report crash detection and attempt recovery (stop/start sequence).

5. **Cold reset from Linux**
   - On Linux, run:
      ```bash
      reboot
      ```
   - A cold reset typically reboots the full platform; you should see MCUboot/TF-M banners again on the CM33 UART.

6. **Warm reset from Linux**
   - On Linux, run:
      ```bash
      echo warm >> /sys/kernel/reboot/mode
      reboot
      ```
   - A warm reset typically restarts the A35 while keeping CM33 running; on the CM33 UART you should see the relevant SCMI notification and RemoteProc/OpenAMP re-init sequence (messages may vary by build options).

## Error Behaviors
If an error occurs during initialization or system configuration at runtime, **LED3 will blink at a 100 ms interval** to indicate the error state.

## Assumptions


## Known Limitations


## Keywords
Security, TFM, Secure, SD Card, Non-Secure

## How to Flash Binaries

1. **Build/Compile the Project**  
  Follow the [Build Procedure](#build-procedure) to generate the required binaries.

2. **Copy and Rename Generated Binaries**
   Before copying, **rename the generated binaries** (`bl2*.stm32`, `ddr_phy*_Signed.bin`, and `tfm-testapp-*_s_ns_Signed.bin`) according to the names specified in the **Required Binaries** section of the relevant flash layout under [Reference Use Cases for MP25-EV1 Board (TestApp Project)](#reference-use-cases-for-mp25-ev1-board-testapp-project).

   Then, navigate to the `bin/` folder and copy the renamed binaries to the following paths:
   ```
   <OSTL-IMAGE-PATH>/images/stm32mp2-m33td/arm-trusted-firmware-m/bl2
   ```
   - Place the renamed `bl2` binary in this directory.
   ```
   <OSTL-IMAGE-PATH>/images/stm32mp2-m33td/m33-firmware
   ```
   - Place the renamed `ddr_phy` and `tfm-testapp` binaries in this directory.

3. **Modify Flashlayout (TSV)**  
   Select the relevant TSV file under `<OSTL-IMAGE-PATH>/images/stm32mp2-m33td/flashlayout_st-image-weston/optee` as per specific dev mode (see [Flash Layouts for MP25-EV1 Board](#flash-layouts-for-mp25-ev1-board)).
   And then modify the TSV file, by replacing the existing TF-M NS signed binary name with the name of your renamed `tfm-testapp-..._s_ns_Signed.bin` that corresponds to your chosen boot mode.

4. **Connect the Device**  
  Use a Type-C cable to connect the device to the Type-C connector.

5. **Flash the Image**  
  Refer to the [STM32 MPU wiki](https://wiki.st.com/stm32mpu/) for detailed flashing instructions.

6. **Perform a Power-On Reset**  
  After flashing, perform a power-on reset to complete the process.

---

## Different Boot Device Modes for MP25-EV1 Board

The MP25-EV1 board supports the following boot device modes:

1. **emmc_emmc**:  
   - MCUBOOT/TFM(S/NS) → eMMC  
   - OSTL → eMMC  

2. **nor_emmc**:  
   - MCUBOOT/TFM(S/NS) → Serial NOR  
   - OSTL → eMMC  

3. **nor_nor_sdcard**:  
   - MCUBOOT/TFM(S/NS) → Serial NOR  
   - TF-A/FIP → Serial NOR  
   - OSTL filesystem → SD card  

4. **nor_sdcard**:  
   - MCUBOOT/TFM(S/NS) → Serial NOR  
   - OSTL → SD card  

5. **sdcard_sdcard**:  
   - MCUBOOT/TFM(S/NS) → SD card  
   - OSTL → SD card  

---

## Flash Layouts for MP25-EV1 Board

For the MP25-EV1 board, five TSV flavors are provided to flash under the M33TDCID profile:

- `FlashLayout_emmc_stm32mp257f-ev1-cm33tdcid-ostl-optee.tsv`
- `FlashLayout_nor-emmc_stm32mp257f-ev1-cm33tdcid-ostl-optee.tsv`
- `FlashLayout_nor-nor-sdcard_stm32mp257f-ev1-cm33tdcid-ostl-optee.tsv`
- `FlashLayout_sdcard_stm32mp257f-ev1-cm33tdcid-ostl-optee.tsv`
- `FlashLayout_nor-sdcard_stm32mp257f-ev1-cm33tdcid-ostl-optee.tsv`

**Note**: Each TSV requires three sets of binaries. Refer to the [Reference Use Cases for MP25-EV1 Board (TestApp Project)](#reference-use-cases-for-mp25-ev1-board-testapp-project) section for details.

---

## How to Generate Binaries for Different Boot Modes

1. **Configure and Build TFM Secure**:  
  Use specific CMake options defined for each boot mode as described in the **Required CMake Options** section under [Reference Use Cases for MP25-EV1 Board (TestApp Project)](#reference-use-cases-for-mp25-ev1-board-testapp-project).  

2. **Build TestApp Project**:  
   Compile the TestApp project after building the TFM secure binaries.  

3. **Rename Generated Binaries**:  
   Rename the binaries in the `bin/` folder to match the required binaries for the specific TSV as described in the **Required Binaries** section under [Reference Use Cases for MP25-EV1 Board (TestApp Project)](#reference-use-cases-for-mp25-ev1-board-testapp-project).  

---

## Reference Use Cases for MP25-EV1 Board (TestApp Project)

### 1. FlashLayout_emmc_stm32mp257f-ev1-cm33tdcid-ostl-optee.tsv (emmc_emmc Boot Mode)

**Required Binaries**:  
- `bl2-stm32mp257f-ev1-cm33tdcid-ostl-emmc.stm32`  
- `ddr_phy-stm32mp257f-ev1-cm33tdcid-ostl-emmc_Signed.bin`  
- `tfm-testapp-stm32mp257f-ev1-cm33tdcid-ostl-emmc-emmc_s_ns_Signed.bin`  

**Required CMake Options**:  
```bash
-DSTM32_BOOT_DEV=sdmmc2  # eMMC
-DDTS_BOARD_BL2=stm32mp2/m33-td/mcuboot/stm32mp257f-ev1-cm33tdcid-ostl-emmc-bl2.dts
-DDTS_BOARD_S=stm32mp2/m33-td/tfm/stm32mp257f-ev1-cm33tdcid-ostl-emmc-s.dts
-DDTS_BOARD_NS=stm32mp2/m33-td/tfm/stm32mp257f-ev1-cm33tdcid-ostl-ns.dts
```

---

### 2. FlashLayout_nor-emmc_stm32mp257f-ev1-cm33tdcid-ostl-optee.tsv (nor_emmc Boot Mode)

**Required Binaries**:  
- `bl2-stm32mp257f-ev1-cm33tdcid-ostl-nor.stm32`  
- `ddr_phy-stm32mp257f-ev1-cm33tdcid-ostl-nor_Signed.bin`  
- `tfm-testapp-stm32mp257f-ev1-cm33tdcid-ostl-nor-emmc_s_ns_Signed.bin`  

**Required CMake Options**:  
```bash
-DSTM32_BOOT_DEV=ospi  # Serial NOR
-DDTS_BOARD_BL2=stm32mp2/m33-td/mcuboot/stm32mp257f-ev1-cm33tdcid-ostl-snor-bl2.dts
-DDTS_BOARD_S=stm32mp2/m33-td/tfm/stm32mp257f-ev1-cm33tdcid-ostl-emmc-s.dts
-DDTS_BOARD_NS=stm32mp2/m33-td/tfm/stm32mp257f-ev1-cm33tdcid-ostl-ns.dts
```

---

### 3. FlashLayout_nor-nor-sdcard_stm32mp257f-ev1-cm33tdcid-ostl-optee.tsv (nor_nor_sdcard Boot Mode)

**Required Binaries**:  
- `bl2-stm32mp257f-ev1-cm33tdcid-ostl-nor.stm32`  
- `ddr_phy-stm32mp257f-ev1-cm33tdcid-ostl-nor_Signed.bin`  
- `tfm-testapp-stm32mp257f-ev1-cm33tdcid-ostl-nor-nor_s_ns_Signed.bin`  

**Required CMake Options**:  
```bash
-DSTM32_BOOT_DEV=ospi  # Serial NOR
-DDTS_BOARD_BL2=stm32mp2/m33-td/mcuboot/stm32mp257f-ev1-cm33tdcid-ostl-snor-bl2.dts
-DDTS_BOARD_S=stm32mp2/m33-td/tfm/stm32mp257f-ev1-cm33tdcid-ostl-snor-s.dts
-DDTS_BOARD_NS=stm32mp2/m33-td/tfm/stm32mp257f-ev1-cm33tdcid-ostl-ns.dts
-DTFM_PARTITION_PROTECTED_STORAGE=OFF     #For this dev mode , Protected Storage feature needs to be disabled 
```

---

### 4. FlashLayout_nor-sdcard_stm32mp257f-ev1-cm33tdcid-ostl-optee.tsv (nor_sdcard Boot Mode)

**Required Binaries**:  
- `bl2-stm32mp257f-ev1-cm33tdcid-ostl-nor.stm32`  
- `ddr_phy-stm32mp257f-ev1-cm33tdcid-ostl-nor_Signed.bin`  
- `tfm-testapp-stm32mp257f-ev1-cm33tdcid-ostl-nor-sdcard_s_ns_Signed.bin`  

**Required CMake Options**:  
```bash
-DSTM32_BOOT_DEV=ospi  # Serial NOR
-DDTS_BOARD_BL2=stm32mp2/m33-td/mcuboot/stm32mp257f-ev1-cm33tdcid-ostl-snor-bl2.dts
-DDTS_BOARD_S=stm32mp2/m33-td/tfm/stm32mp257f-ev1-cm33tdcid-ostl-sdcard-s.dts
-DDTS_BOARD_NS=stm32mp2/m33-td/tfm/stm32mp257f-ev1-cm33tdcid-ostl-ns.dts
```

---

### 5. FlashLayout_sdcard_stm32mp257f-ev1-cm33tdcid-ostl-optee.tsv (sdcard_sdcard Boot Mode)

**Required Binaries**:  
- `bl2-stm32mp257f-ev1-cm33tdcid-ostl-sdcard.stm32`  
- `ddr_phy-stm32mp257f-ev1-cm33tdcid-ostl-sdcard_Signed.bin`  
- `tfm-testapp-stm32mp257f-ev1-cm33tdcid-ostl-sdcard-sdcard_s_ns_Signed.bin`  

**Required CMake Options**:  
```bash
-DSTM32_BOOT_DEV=sdmmc1  # SD card
-DDTS_BOARD_BL2=stm32mp2/m33-td/mcuboot/stm32mp257f-ev1-cm33tdcid-ostl-sdcard-bl2.dts
-DDTS_BOARD_S=stm32mp2/m33-td/tfm/stm32mp257f-ev1-cm33tdcid-ostl-sdcard-s.dts
-DDTS_BOARD_NS=stm32mp2/m33-td/tfm/stm32mp257f-ev1-cm33tdcid-ostl-ns.dts
```
