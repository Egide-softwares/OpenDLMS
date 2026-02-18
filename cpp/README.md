# OpenDLMS
An open source DLMS-COSEM library

## Setup

### 🐧 Linux (Ubuntu/Debian)
For your 4-month sprint, use the built-in package manager to get the standard toolchain.

1. Install GCC or Clang
The build-essential package includes GCC, make, and other necessary tools.

```sh
sudo apt update
sudo apt install build-essential  # Installs GCC
# OR for Clang
sudo apt install clang
```

2. Install CMake

```sh
sudo apt install cmake
# Verify the version
cmake --version
```

### 🪟 Windows
On Windows, the most robust way to get a C++17 compiler is through Visual Studio.

1. Install MSVC (via Visual Studio 2019/2022)
Download the Visual Studio Community (free) installer.

In the installer, select the workload: "Desktop development with C++".

This automatically installs the MSVC compiler, the Windows SDK, and a version of CMake.

2. Install Standalone CMake (Optional but Recommended)
Download the .msi installer from cmake.org.

Crucial: During installation, select "Add CMake to the system PATH for all users". This allows you to run cmake from any terminal.

### 🍎 macOS
macOS uses the Clang compiler by default via Apple's Command Line Tools.

1. Install Clang & Build Tools
Open your terminal and run:

```sh
xcode-select --install
```

2. Install CMake
The easiest way is using Homebrew:

```sh
brew install cmake
```

### Install c++ asio

On Ubuntu, you can install the standalone version (non-Boost) easily:

```sh
sudo apt install libasio-dev
```

## Build Library
Now that the files exist, go back to your terminal and run:

```sh
cd build
cmake ..         # This should now succeed with "Configuring done"
cmake --build .  # This will compile your first (empty) library
```
## Summary of Handshake Logic
Phase,HDLC Requirement,Wrapper Requirement
Link,SNRM → UA (Parsed here),None (TCP Only)
Association,AARQ → AARE,AARQ → AARE
Data,GET → Response,GET → Response

## Running the Tests
To compile and run, follow these steps from your build directory:

```sh
# 1. Compile
cmake ..
make

# 2. Start Simulator for HDLC (No -w)
# java -jar gurux-dlms-simulator-java.jar -p 4050 -N 1 -t Verbose -x xml/crystal.xml

# 3. In a new terminal, run the HDLC client
./tcp_hdlc_client

# --- SWITCHING MODES ---

# 4. Stop Simulator and Restart for Wrapper (With -w)
# java -jar gurux-dlms-simulator-java.jar -p 4051 -N 1 -t Verbose -w -x xml/crystal.xml

# 5. Run the Wrapper client
./tcp_wrapper_client
```