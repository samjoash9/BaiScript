const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const fs = require('fs');
const { spawn } = require('child_process');

let mainWindow;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1400,
    height: 900,
    frame: false, // Remove default window frame
    titleBarStyle: 'hidden', // Hide title bar (macOS)
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      nodeIntegration: false,
      contextIsolation: true,
    },
  });

  // Remove menu bar
  mainWindow.setMenuBarVisibility(false);

  // Load the app
  // Check if we're in development mode (Vite dev server running)
  const isDev = process.env.NODE_ENV === 'development' || !app.isPackaged;

  if (isDev) {
    // Try to load from Vite dev server
    mainWindow.loadURL('http://localhost:3000');
    // DevTools are not opened automatically

    // If dev server is not available, show error
    mainWindow.webContents.on('did-fail-load', () => {
      console.log('Dev server not running. Please run "npm run dev" in another terminal.');
    });
  } else {
    // Production: load from build folder
    mainWindow.loadFile(path.join(__dirname, '../build/index.html'));
  }
}

// Remove default menu bar
const { Menu } = require('electron');
Menu.setApplicationMenu(null);

app.whenReady().then(() => {
  createWindow();

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

// IPC handler to run the compiler
ipcMain.handle('run-compiler', async (event, sourceCode) => {
  return new Promise((resolve, reject) => {
    try {
      // Get the directory where main.exe is located
      // Adjust this path based on where your main.exe is located
      const exePath = path.join(__dirname, '../main.exe');
      const inputPath = path.join(__dirname, '../input.txt');
      const outputPrintPath = path.join(__dirname, '../output_print.txt');
      const outputAssemblyPath = path.join(__dirname, '../output_assembly.txt');
      const outputMachinePath = path.join(__dirname, '../output_machine.txt');

      // Write source code to input.txt
      fs.writeFileSync(inputPath, sourceCode, 'utf-8');

      // Run main.exe
      const compilerProcess = spawn(exePath, [], {
        cwd: path.dirname(exePath),
        stdio: ['pipe', 'pipe', 'pipe'],
      });

      let stdout = '';
      let stderr = '';

      compilerProcess.stdout.on('data', (data) => {
        stdout += data.toString();
      });

      compilerProcess.stderr.on('data', (data) => {
        stderr += data.toString();
      });

      compilerProcess.on('close', (code) => {
        const safeRead = (filePath) => {
          try {
            if (fs.existsSync(filePath)) {
              return fs.readFileSync(filePath, 'utf-8');
            }
          } catch (err) {
            console.error(`Error reading ${filePath}:`, err);
          }
          return '';
        };

        resolve({
          success: code === 0,
          exitCode: code,
          stdout,
          stderr,
          outputs: {
            print: safeRead(outputPrintPath),
            assembly: safeRead(outputAssemblyPath),
            machine: safeRead(outputMachinePath),
          },
        });
      });

      compilerProcess.on('error', (error) => {
        reject({
          success: false,
          error: error.message,
        });
      });
    } catch (error) {
      reject({
        success: false,
        error: error.message,
      });
    }
  });
});

// Window control handlers
ipcMain.handle('window-minimize', () => {
  if (mainWindow) {
    mainWindow.minimize();
  }
});

ipcMain.handle('window-maximize', () => {
  if (mainWindow) {
    if (mainWindow.isMaximized()) {
      mainWindow.unmaximize();
    } else {
      mainWindow.maximize();
    }
  }
});

ipcMain.handle('window-close', () => {
  if (mainWindow) {
    mainWindow.close();
  }
});

ipcMain.handle('window-is-maximized', () => {
  return mainWindow ? mainWindow.isMaximized() : false;
});

