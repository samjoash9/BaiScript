const { app, BrowserWindow, ipcMain, Menu } = require('electron');
const path = require('path');
const fs = require('fs');
const { spawn } = require('child_process');

let mainWindow;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1400,
    height: 900,
    minWidth: 900,
    minHeight: 600,
    frame: false,
    titleBarStyle: 'hidden',
    resizable: true,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      nodeIntegration: false,
      contextIsolation: true,
    },
  });

  Menu.setApplicationMenu(null);

  const isDev = process.env.NODE_ENV === 'development' || !app.isPackaged;

  if (isDev) {
    const devPort = process.env.VITE_DEV_PORT || 3000;
    const devUrl = `http://localhost:${devPort}`;
    mainWindow.loadURL(devUrl);

    mainWindow.webContents.on('did-fail-load', () => {
      console.error(
        `Failed to load dev server at ${devUrl}. Run "npm run dev" in another terminal.`
      );
    });

    mainWindow.webContents.openDevTools({ mode: 'detach' });
  } else {
    const indexHtml = path.join(__dirname, '../build/index.html');
    mainWindow.loadFile(indexHtml);
  }
}

app.whenReady().then(() => {
  createWindow();

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) createWindow();
  });
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});

// --------------------- IPC Handlers ---------------------

ipcMain.handle('run-compiler', async (event, sourceCode) => {
  return new Promise((resolve, reject) => {
    try {
      const isDev = process.env.NODE_ENV === 'development' || !app.isPackaged;

      // Paths
      const exePath = isDev
        ? path.join(__dirname, '../main.exe')           // Dev mode
        : path.join(process.resourcesPath, 'main.exe'); // Production

      // Folder to store input/output files
      const tempDir = isDev
        ? path.join(__dirname, '..')    // Project root in dev
        : app.getPath('userData');      // UserData folder in production

      const inputPath = path.join(tempDir, 'input.txt');
      const outputPrintPath = path.join(tempDir, 'output_print.txt');
      const outputAssemblyPath = path.join(tempDir, 'output_assembly.txt');
      const outputMachinePath = path.join(tempDir, 'output_machine.txt');

      // Write source code to input.txt
      fs.writeFileSync(inputPath, sourceCode, 'utf-8');

      // Run compiler
      const compilerProcess = spawn(exePath, [], {
        cwd: tempDir, // Important: compiler reads/writes txt files here
        stdio: ['pipe', 'pipe', 'pipe'],
      });

      let stdout = '';
      let stderr = '';

      compilerProcess.stdout.on('data', (data) => (stdout += data.toString()));
      compilerProcess.stderr.on('data', (data) => (stderr += data.toString()));

      compilerProcess.on('close', (code) => {
        const safeRead = (filePath) => {
          try {
            return fs.existsSync(filePath) ? fs.readFileSync(filePath, 'utf-8') : '';
          } catch (err) {
            console.error(`Error reading ${filePath}:`, err);
            return '';
          }
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
        reject({ success: false, error: error.message });
      });
    } catch (error) {
      reject({ success: false, error: error.message });
    }
  });
});

// ----------------- Window Controls -----------------

ipcMain.handle('window-minimize', () => mainWindow?.minimize());
ipcMain.handle('window-maximize', () => {
  if (!mainWindow) return;
  mainWindow.isMaximized() ? mainWindow.unmaximize() : mainWindow.maximize();
});
ipcMain.handle('window-close', () => mainWindow?.close());
ipcMain.handle('window-is-maximized', () => mainWindow?.isMaximized() || false);
