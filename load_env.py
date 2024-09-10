import os
from dotenv import load_dotenv

load_dotenv()

env = os.environ

with open('include/env.h', 'w') as f:
    f.write(f'#define WIFI_SSID "{env["WIFI_SSID"]}"\n')
    f.write(f'#define WIFI_PASSWORD "{env["WIFI_PASSWORD"]}"\n')




    