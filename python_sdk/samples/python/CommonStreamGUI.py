import os
import sys
from PyQt5.QtGui import QIcon
from PyQt5.QtWidgets import QApplication
from CSMainDialog.mainDlg import main_Dialog

if __name__ == '__main__':
	app = QApplication(sys.argv)
	icon = os.path.dirname(os.path.abspath(__file__)) + '/CSMainDialog/imperx.ico'
	app.setWindowIcon(QIcon(icon))
	w = main_Dialog()
	w.show()
	app.exec_()
	#sys.exit()
