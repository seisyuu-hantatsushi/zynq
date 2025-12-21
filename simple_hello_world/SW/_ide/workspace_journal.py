# 2025-11-30T10:16:07.725442031
import vitis

client = vitis.create_client()
client.set_workspace(path="SW")

platform = client.create_platform_component(name = "platform",hw_design = "$COMPONENT_LOCATION/../../HW/hello_world/rk_simple_hw.xsa",os = "standalone",cpu = "ps7_cortexa9_0",domain_name = "standalone_ps7_cortexa9_0",compiler = "gcc")

platform = client.get_component(name="platform")
status = platform.build()

comp = client.create_app_component(name="hello_world",platform = "$COMPONENT_LOCATION/../platform/export/platform/platform.xpfm",domain = "standalone_ps7_cortexa9_0",template = "hello_world")

comp = client.get_component(name="hello_world")
comp.build()

vitis.dispose()

