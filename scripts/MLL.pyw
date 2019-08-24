import os, sys

### How to make exe with pyinstaller (pip install pyinstaller)
# (the path to the image probably doesn't need to be a full path)
# Copy the script elsewhere first, because it will create dist/ etc in the repo
# pyinstaller --onefile --noconsole -i C:\Users\dboun\Desktop\tmp\mll_icon2.ico MLL.pyw

### Find script or exe directory
application_path = ''
if getattr(sys, 'frozen', False):
    # If the application is run as a bundle, the pyInstaller bootloader
    # extends the sys module by a flag frozen=True and sets the app 
    # path into variable _MEIPASS'.
    application_path = sys.executable
else:
    application_path = os.path.dirname(os.path.abspath(__file__))

application_path = application_path.replace('\\', '/')
if application_path.endswith('/'):
	application_path = application_path[:-1]

### Run MLL
os.startfile( '\"' + 
	os.path.dirname(application_path) + 
	'/files/MLL Semi-Automatic Segmentation.exe\"'
)