import os
import shutil
import vitis

WS = os.path.abspath("./")
PLATFORM_NAME = "platform"
APP_NAME = "hello_world"

XSA_PATH = os.path.abspath("../HW/hello_world/rk_simple_hw.xsa")

# platform.xpfm は platform component 配下に生成される（既定の出力場所想定）
XPFM_PATH = os.path.join(WS, PLATFORM_NAME, "export", PLATFORM_NAME, "platform.xpfm")

# True にすると platform/app を削除して作り直す（完全rebuild）
CLEAN_RECREATE = False

def rm_if_exists(path: str):
    if os.path.isdir(path):
        shutil.rmtree(path)

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
            domain_name="standalone_ps7_cortexa9_0",
            compiler="gcc",
        )

    # platform をビルド（既にできてても基本は通してOK）
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
            domain="standalone_ps7_cortexa9_0",
            template="hello_world",
        )

    print("[INFO] Building app...")
    app.build()

finally:
    vitis.dispose()
