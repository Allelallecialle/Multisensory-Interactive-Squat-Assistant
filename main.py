import sys
from PySide6.QtWidgets import QApplication
from UI import SquatUI


# ----------------- Main -----------------
if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = SquatUI()
    win.show()
    sys.exit(app.exec())
