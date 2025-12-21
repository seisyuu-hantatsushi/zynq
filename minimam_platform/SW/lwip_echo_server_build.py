import os
import shutil
import vitis

WS = os.path.abspath("./")
PLATFORM_NAME = "minimam_platform"
APP_NAME = "lwip_echo_server"
DOMAIN_NAME = "standalone_ps7_cortexa9_0"
BSP_LIBS = ["lwip220"]

XSA_PATH = os.path.abspath("../HW/minimam_platform/minimam_platform.xsa")

# platform.xpfm は platform component 配下に生成される（既定の出力場所想定）
XPFM_PATH = os.path.join(WS, PLATFORM_NAME, "export", PLATFORM_NAME, "platform.xpfm")

# True にすると platform/app を削除して作り直す（完全rebuild）
CLEAN_RECREATE = False

def rm_if_exists(path: str):
    if os.path.isdir(path):
        shutil.rmtree(path)

def enable_bsp_libs(platform, domain_name: str, libs):
    try:
        domain = platform.get_domain(name=domain_name)
    except Exception as exc:
        raise RuntimeError(f"Failed to get domain '{domain_name}': {exc}") from exc

    try:
        current_libs = set(domain.get_libs())
    except Exception:
        current_libs = set()

    last_exc = None
    for method_name in (
        "set_lib",
        "set_libs",
        "set_bsp_libs",
        "add_lib",
        "add_library",
        "add_bsp_lib",
    ):
        method = getattr(domain, method_name, None)
        if not method:
            continue
        try:
            if method_name.startswith("add"):
                for lib in libs:
                    if lib in current_libs:
                        continue
                    method(lib)
            elif method_name == "set_lib":
                for lib in libs:
                    if lib in current_libs:
                        continue
                    method(lib)
            else:
                method(libs)
            print(f"[INFO] Enabled BSP libs on {domain_name}: {', '.join(libs)}")
            return
        except Exception as exc:
            last_exc = exc

    raise RuntimeError(
        f"Failed to enable BSP libs on domain '{domain_name}'. "
        f"No compatible API found or last error: {last_exc}"
    )

client = vitis.create_client()
client.set_workspace(path=WS)

try:
    if CLEAN_RECREATE:
        rm_if_exists(os.path.join(WS, PLATFORM_NAME))
        rm_if_exists(os.path.join(WS, APP_NAME))

    # ---- platform: 既存なら get、無ければ create ----
    try:
        platform = client.get_component(name=PLATFORM_NAME)
        print(f"[INFO] Reusing existing platform component: {PLATFORM_NAME}")
    except Exception:
        print(f"[INFO] Creating platform component: {PLATFORM_NAME}")
        platform = client.create_platform_component(
            name=PLATFORM_NAME,
            hw_design=XSA_PATH,
            os="standalone",
            cpu="ps7_cortexa9_0",
            domain_name=DOMAIN_NAME,
            compiler="gcc",
        )

    # platform をビルド（既にできてても基本は通してOK）
    enable_bsp_libs(platform, DOMAIN_NAME, BSP_LIBS)
    print("[INFO] Building platform...")
    platform.build()

    # ---- app: 既存なら get、無ければ create ----
    try:
        app = client.get_component(name=APP_NAME)
        print(f"[INFO] Reusing existing app component: {APP_NAME}")
    except Exception:
        print(f"[INFO] Creating app component: {APP_NAME}")
        if not os.path.exists(XPFM_PATH):
            raise FileNotFoundError(f"platform.xpfm not found: {XPFM_PATH}")

        app = client.create_app_component(
            name=APP_NAME,
            platform=XPFM_PATH,
            domain=DOMAIN_NAME,
            template="hello_world",
        )

    print("[INFO] Building app...")
    app.build()

finally:
    vitis.dispose()
