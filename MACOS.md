## Debugging under Xcode

Build the plugin

```
cd <OpenCPN build directory>
cmake -GXcode ..
mkdir -p Debug/OpenCPN.app/Contents/PlugIns
mkdir Debug/OpenCPN.app/Contents/PlugIns/DashboardSK
ln -s <DashboardSK_pi build directory>/Debug/libDashboardSK_pi.dylib Debug/OpenCPN.app/Contents/PlugIns/libDashboardSK_pi.dylib
ln -s <DashboardSK_pi source directory>/data/* Debug/OpenCPN.app/Contents/PlugIns/DashboardSK
```

Build OpenCPN

Run OpenCPN from inside Xcode
