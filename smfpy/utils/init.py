from . import C_init

def init_glog(program_name: str = "default_program", log_dir: str = "", 
              log_level: int = 0, stderr_log_level: int = 3) -> None:
    """
    Initialize Google Logging.

    Args:
        program_name (str, optional): The name of the program. Default is "default_program".
        log_dir (str, optional): The directory where log files will be saved. Default is "/tmp".
        log_level (int, optional): The log level (0 for INFO, 1 for WARNING, 2 for ERROR, 3 for FATAL). Default is 0.
        log_level (int, optional): The stderr log level (0 for INFO, 1 for WARNING, 2 for ERROR, 3 for FATAL),
                                   means which level will be output to stderr(terminal). Default is 3.
    """
    C_init.init_glog(program_name, log_dir, log_level, stderr_log_level)