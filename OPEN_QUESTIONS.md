# Open Questions & Decisions for Owner

1. **Commercial or AGPLv3 Release:** Decide whether LadderMono will be distributed under AGPLv3 or under a JUCE commercial license (Starter tier is free for sub-$20k revenue).
2. **Brand & Manufacturer Name:** Defaulted to `YourName` / `Lmno` / `Ynam`. Can be customized via CMake variables `PRODUCT_NAME`, `COMPANY_NAME`, `PLUGIN_CODE`, and `MANUFACTURER_CODE`.
3. **Apple Developer Signing & Notarization:** Required for public macOS distribution outside local machine (Developer ID Application certificate and App Store Connect API keys).
