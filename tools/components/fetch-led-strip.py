Import("env")
import os
import subprocess
import re

env_name = env["PIOENV"]
pio_libdeps = env["PROJECT_LIBDEPS_DIR"]
led_strip_dir = os.path.join(pio_libdeps, env_name, "led_strip")

if not os.path.isdir(led_strip_dir):
    # Clone idf-extra-components temporarily to get led_strip
    temp_dir = os.path.join(pio_libdeps, env_name, "idf-extra-components-temp")
    
    subprocess.check_call([
        "git", "clone",
        "--filter=blob:none",
        "--no-checkout",
        "https://github.com/espressif/idf-extra-components.git",
        temp_dir
    ])

    subprocess.check_call(
        ["git", "sparse-checkout", "init", "--cone"],
        cwd=temp_dir
    )

    subprocess.check_call(
        ["git", "sparse-checkout", "set", "led_strip"],
        cwd=temp_dir
    )

    subprocess.check_call(
        ["git", "checkout"],
        cwd=temp_dir
    )
    
    # Move led_strip out of temp directory
    import shutil
    temp_led_strip = os.path.join(temp_dir, "led_strip")
    shutil.move(temp_led_strip, led_strip_dir)
    
    # Remove temporary directory
    shutil.rmtree(temp_dir)
    
    # Extract version from idf_component.yml
    component_yml_path = os.path.join(led_strip_dir, "idf_component.yml")
    version = "3.0.2"  # Default fallback
    
    if os.path.isfile(component_yml_path):
        with open(component_yml_path, "r") as f:
            for line in f:
                match = re.match(r'^version:\s*["\']?([0-9]+\.[0-9]+\.[0-9]+)["\']?', line)
                if match:
                    version = match.group(1)
                    break
    
    # Create library.json for PlatformIO to recognize the component
    library_json_path = os.path.join(led_strip_dir, "library.json")
    library_json_content = f"""{{
  "name": "led_strip",
  "version": "{version}",
  "description": "ESP-IDF led_strip component for WS2812, SK6812, WS2811 addressable LEDs",
  "keywords": "led, ws2812, sk6812, addressable, rgb, rmt, spi",
  "repository": {{
    "type": "git",
    "url": "https://github.com/espressif/idf-extra-components.git"
  }},
  "license": "Apache-2.0",
  "frameworks": "espidf, arduino",
  "platforms": "espressif32",
  "build": {{
    "flags": [
      "-I interface",
      "-I include"
    ],
    "srcDir": "src"
  }}
}}"""
    
    with open(library_json_path, "w") as f:
        f.write(library_json_content)
