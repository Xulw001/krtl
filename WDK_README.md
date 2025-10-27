## Introduction
this library is adapted from FindWDK and wdk_template, 
but remove something not used like wdf, and older wdk version support.


## Support
- support generating driver and static library project
- support driver signature by local certificates


## Usage
#### 1. Compile Options Backup and Restore
when use this library, 
```cmake
## clean default compile options within Visual Studio to fix compile error
wdk_clean_default_options()
## restore default compile options when solution contains user mode project
wdk_restore_default_options()
```

#### 2. Build Static lib
The command adds a kernel static library called `<target>`:
```cmake
wdk_add_library(<target>
    [WINVER <winver_version>]
    [NTDDI_VERSION <ntddi_version>]
    source1 [source2 ...]
    )
```
Options:
- `WINVER <winver_version>` -- Specify WINVER version
- `NTDDI_VERSION <ntddi_version>` -- Specify NTDDI_VERSION

#### 3. Build WDM Driver
The command adds a wdm driver called `<target>` within specified signature:
```cmake
wdk_add_library(<target>
    [WINVER <winver_version>]
    [NTDDI_VERSION <ntddi_version>]
    [SIGN_STORE <windows_sign_store>]
    [SIGN_NAME <windows_sign_name>]
    source1 [source2 ...]
    )
```
Options:
- `WINVER <winver_version>` -- Specify WINVER version
- `NTDDI_VERSION <ntddi_version>` -- Specify NTDDI_VERSION
- `SIGN_STORE <windows_sign_store>` -- Specify the Store to open when searching for the cert. The default is the "MY" Store.
- `SIGN_NAME <windows_sign_name>` -- Specify the Subject Name of the signing cert.if without this, will select the best signing cert automatically.

## Reference
- [FindWDK](https://github.com/SergiusTheBest/FindWDK.git)
- [wdk_template](https://github.com/build-cpp/wdk_template.git)