import usb.core
import usb.util

devices = usb.core.find(find_all=True)

i = 0
for dev in devices:
    try:
        print(f'Device {i}----------------------------------------------------------------')
        # print(dev)
        
        print(f'vendor id: {hex(dev.idVendor)}')
        print(f'product id: {hex(dev.idProduct)}')
        
        print(f'manufacturer: {dev.manufacturer}')
        print(f'product: {dev.product}')
        print(f'serial_number: {dev.serial_number}')
        
    except Exception:
        print('error')
        
    i += 1