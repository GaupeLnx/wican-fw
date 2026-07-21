
# [Documentation](https://wambs.github.io/wican-fw/) | [Firmware updates](https://github.com/wambs/wican-fw/releases/) | [Discord server](https://discord.com/invite/2hpHVDmyfw)

## Order on [**Mouser**](https://www.mouser.com/c/?m=MeatPi) or [**Crowd Supply!**](https://www.crowdsupply.com/meatpi-electronics)

### What is the WiCAN Pro?
At its core, the WiCAN Pro is a powerful, ESP32-S3-based OBD2 and CAN bus interface. It plugs directly into your vehicle's diagnostic port and acts as a bridge between your car's internal networks and your Wi-Fi or Bluetooth devices. It supports standard OBD2 protocols (via an ELM327-compatible interpreter) as well as raw CAN bus monitoring and injection.

### Why the Forked Edition?
The original WiCAN firmware was great for basic polling, but it relied on a "flat list" architecture. Every PID was polled blindly on a timer, which often resulted in network spam, slow update rates, and unnecessary battery drain when the car was off.

This Forked Edition completely rewrites the automation and networking engines. It transforms the WiCAN Pro from a simple OBD scanner into a highly intelligent, edge-computing telemetry node. If you wish to try out the WiCAN Pro forked firmware see the release Wambs Forked Firmware Releases

### Key Features
Vehicle Groups & Advanced PID Management (The Game Changer)
The old flat list of PIDs is gone, replaced by Vehicle Groups. Groups were specifically created to allow you to access different vehicle modules based on dedicated polling periods, optimizing CAN bus traffic and maximizing response efficiency.

- Dual-Database Profile Ecosystem: You are no longer limited to a static, hardcoded firmware database. The UI now features a dedicated Get Vehicle Group DB button that fetches the absolute latest community-maintained profiles from an ultra-fast, indexed GitHub repository. The profiles are lazy-loaded on demand to keep the UI lightning fast.
- Smart Profile Merging & HA Bundling: When loading new vehicle data, the UI intelligently prompts you to safely merge the new groups into your existing workspace or replace them entirely. Furthermore, community profiles now act as a "briefcase"—if a user bundled a Home Assistant YAML configuration with their profile, the - WiCan will automatically extract and download it straight to your PC.
- Interactive Group Console: Testing your custom formulas is now effortless. Every Vehicle Group features a dedicated "Play/Test" button that launches a live terminal directly in your browser. It polls the ECU in real-time, displays the raw CAN hex responses, and instantly validates your math expressions before you deploy them.
- Advanced Editor & Drag-and-Drop: The UI now features a robust visual editor. You can easily copy/paste (clone) PIDs, edit parameters on the fly, and seamlessly drag and drop PIDs between different Vehicle Groups to fine-tune your architecture.
- Smart Polling & Conditional Logic: Tell a group to only poll when the "Engine is Running," when the "Battery Voltage" is above a specific threshold, or trigger it via an on-demand custom gatekeeper PID. This completely halts unnecessary traffic when the car is parked.
- Zero Network Spam: Groups package all their PID responses into a single JSON payload and push it to a unified MQTT_Grp topic the exact millisecond the batch finishes, drastically reducing MQTT broker congestion.


