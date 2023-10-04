from . import C_uart

class SensorDeviceType:
    SoilSensor = C_uart.SensorDeviceType.SoilSensor
    AirSensor = C_uart.SensorDeviceType.AirSensor

def reset_device() -> bool:
    """
    Reset connected devices to address 0.
    
    Returns:
        bool: True if reset was successful, False otherwise.
    """
    return C_uart.reset_device()

def register_device(device_type: SensorDeviceType, device_id: int) -> bool:
    """
    Register a new device with the specified type and ID.

    Args:
        device_type (SensorDeviceType): The type of the device (SoilSensor or AirSensor).
        device_id (int): The ID of the device to register.

    Returns:
        bool: True if registration was successful, False otherwise.
    """
    return C_uart.register_device(device_type, device_id)

def remove_device(device_id: int) -> bool:
    """
    Remove a device with the specified ID from the registry.

    Args:
        device_id (int): The ID of the device to remove.

    Returns:
        bool: True if removal was successful, False otherwise.
    """
    return C_uart.remove_device(device_id)

def list_all_devices() -> list:
    """
    List all registered devices with their addresses and types.

    Returns:
        list: A list of tuples containing (device_id, device_type) for each registered device.
    """
    return C_uart.list_all_devices()

def clear_all_devices() -> bool:
    """
    Clear the registry, removing all registered devices.

    Returns:
        bool: True if clearing was successful, False otherwise.
    """
    return C_uart.clear_all_devices()
