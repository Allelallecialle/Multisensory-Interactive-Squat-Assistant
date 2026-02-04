import sys
from PySide6.QtWidgets import QApplication
from UI import SquatUI
from serial_communication import SerialController

# ----------------- Main -----------------
if __name__ == "__main__":
    app = QApplication(sys.argv)
    arduino = SerialController()
    win = SquatUI(arduino)
    win.show()
    sys.exit(app.exec())
