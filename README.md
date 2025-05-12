# SAVA

## Operations
- Initialize each sensor driver
- Initialize data handlers
- Initialize flight status 
- Request data from each sensor driver 
- Pass sensor data into the data handlers
- Pass the data handlers into the flight status updater

## Workspace Setup
1. Download VS code
2. Clone the repo
```bash
git clone https://github.com/CURocketEngineering/SAVA.git
```
3. Setup the submodules 
```bash
git submodule init
git submodule update 
```
4. Install the PlatformIO VScode extension: `platformio.platformio-ide`
5. Install the [Cube Programmer](https://www.st.com/en/development-tools/stm32cubeprog.html#get-software) and run it to get all the stm32 drivers you'll need. 