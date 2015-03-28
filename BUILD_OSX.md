BitShares OS X Build Instructions
===============================

0. Old guide: https://github.com/bitsuperlab/bitshares_play/blob/game_layer/BUILD_OSX.md

1. Install XCode and its command line tools by following the instructions here: https://guide.macports.org/#installing.xcode

2. Install Homebrew by following the instructions here: http://brew.sh/

3. Initialize Homebrew:
   ```
   brew doctor
   brew update
   ```

4. Install dependencies:
   ```
   brew install boost cmake git openssl readline
   brew link --force openssl readline
   ```

5. *Optional.* To support importing Bitcoin wallet files:
   ```
   brew install berkeley-db
   ```

6. *Optional.* To use TCMalloc in LevelDB:
   ```
   brew install google-perftools
   ```
   Tip 1: For those who can not download the file with curl due to gfw, refer https://github.com/shadowsocks/shadowsocks/wiki/Using-Shadowsocks-with-Command-Line-Tools
   and http://www.v2ex.com/t/138697
   
   Tip 2: 2.3 or above required. https://github.com/bitsuperlab/bitshares_play/issues/76

7. Clone the DAC PLAY repository:
   ```
   git clone git@github.com:dacsunlimited/dac_play.git
   cd dac_play
   ```

8. Build DAC PLAY:
   ```
   git submodule update --init
   cmake .
   make
   ```

9. *Optional*. Install dependencies for the GUI:
   ```
   brew install node qt5
   ```

10. *Optional*. Build the GUI:
   ```
   cd programs/web_wallet
   npm install .
   cd ../..
   export CMAKE_PREFIX_PATH=/usr/local/opt/qt5/
   cmake -DINCLUDE_QT_WALLET=1 .
   make buildweb
   make
   ```
   Note: By default, the web wallet will not be rebuilt even after pulling new changes. To force the web wallet to rebuild, use `make forcebuildweb`.
   
   Node2: In 'make buildweb' step, if there is any error, you can switch to 'programs/web_wallet' and run 'lineman build' to see more error details. Make sure 'lineman' is installed. 

11. *Optional*. Create GUI installation package:
   ```
   make package
   ```
