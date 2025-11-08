mkdir ../build
cd ../build
cmake -G "Xcode" -DPROJECT_ARCH="ARM64" ..
xcodebuild -project cef.xcodeproj \             
-scheme muon -configuration Debug build
open muon/Debug/muon.app
