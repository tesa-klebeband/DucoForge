# DucoForge
DucoForge is a versatile Duino-Coin miner emulator enabling users to simulate mining rigs made from Arduinos, ESP32s and ESP8266s

## Disclaimer
This project is for educational purposes only. Using this tool to mine on the official Duino-Coin network will probably get you banned. Use at your own risk. I am **not** responsible for any damage caused by this project to your account.

## Building
Install the following packages by running the command suitable for your package manager:
- Debian/Ubuntu
```bash
sudo apt update && sudo apt install -y git make libcurl4-openssl-dev build-essential libssl-dev libsfml-dev rapidjson-dev
```
- Fedora
```bash
sudo dnf install -y git make libcurl-devel gcc-c++ openssl-devel sfml-devel rapidjson-devel
```
- Arch
```bash
sudo pacman -Syu --needed git make curl gcc openssl sfml rapidjson
```
Then clone the repository and build the project:
```bash
git clone https://github.com/tesa-klebeband/DucoForge.git
cd DucoForge
make
```

## Using DucoForge
DucoForge relies on a JSON configuration file to run. An example configuration file is provided in the repository. To run DucoForge, simply run the following command:
```bash
build/ducoforge config.json
```

## License
All files within this repo are released under the GNU GPL V3 License as per the LICENSE file stored in the root of this repo.
