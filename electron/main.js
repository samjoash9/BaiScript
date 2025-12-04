const { app, BrowserWindow, ipcMain, Menu, nativeImage } = require('electron');
const path = require('path');
const fs = require('fs');
const { spawn } = require('child_process');

let mainWindow;

function createWindow() {
  const isDev = process.env.NODE_ENV === 'development' || !app.isPackaged;

  // SIMPLE ICON LOADING
  let icon = null;

  if (isDev) {
    // Development: Look in electron folder
    const devIconPaths = [
      path.join(__dirname, 'icon.ico'),  // Windows
      path.join(__dirname, 'icon.png'),  // Linux
      path.join(__dirname, 'icon.icns'), // macOS
    ];

    for (const iconPath of devIconPaths) {
      if (fs.existsSync(iconPath)) {
        try {
          icon = nativeImage.createFromPath(iconPath);
          if (!icon.isEmpty()) {
            console.log('✓ Loaded dev icon from:', iconPath);
            console.log('Icon size:', icon.getSize());
            break;
          }
        } catch (err) {
          console.log('Failed to load', iconPath, ':', err.message);
        }
      }
    }
  } else {
    // Production: Look in resources folder (where extraResources copies to)
    const prodIconPaths = [
      path.join(process.resourcesPath, 'icon.ico'),  // Windows
      path.join(process.resourcesPath, 'icon.png'),  // Linux
      path.join(process.resourcesPath, 'icon.icns'), // macOS
    ];

    for (const iconPath of prodIconPaths) {
      if (fs.existsSync(iconPath)) {
        try {
          icon = nativeImage.createFromPath(iconPath);
          if (!icon.isEmpty()) {
            console.log('✓ Loaded production icon from:', iconPath);
            console.log('Icon size:', icon.getSize());
            break;
          }
        } catch (err) {
          console.log('Failed to load', iconPath, ':', err.message);
        }
      }
    }
  }

  // If still no icon, let Electron use OS default (set icon to null)
  if (!icon || icon.isEmpty()) {
    console.log('No icon found, using OS default');
    icon = null;  // This lets the OS use its default icon
  }

  mainWindow = new BrowserWindow({
    width: 1600,
    height: 900,
    minWidth: 1170,
    minHeight: 600,
    frame: false,
    titleBarStyle: 'hidden',
    resizable: true,
    icon: icon,  // Can be null - that's okay!
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      nodeIntegration: false,
      contextIsolation: true,
    },
    show: false,
  });

  // Set dock icon on macOS if we have one
  if (icon && !icon.isEmpty() && process.platform === 'darwin') {
    app.dock.setIcon(icon);
  }

  Menu.setApplicationMenu(null);

  if (isDev) {
    // DEVELOPMENT MODE - Wait for Vite dev server
    const devPort = process.env.VITE_DEV_PORT || 3000;
    const devUrl = `http://localhost:${devPort}`;

    console.log('=== DEVELOPMENT MODE ===');
    console.log('Waiting for Vite dev server at:', devUrl);

    // Try to load the dev server, retry if it fails
    const tryLoadDevServer = (retries = 10, delay = 1000) => {
      const attemptLoad = (attempt = 1) => {
        console.log(`Attempt ${attempt}/${retries} to connect to dev server...`);

        mainWindow.loadURL(devUrl).then(() => {
          console.log('✓ Successfully connected to Vite dev server');
          mainWindow.webContents.openDevTools({ mode: 'detach' });
        }).catch((err) => {
          console.log(`✗ Failed to connect (attempt ${attempt}):`, err.message);

          if (attempt < retries) {
            setTimeout(() => attemptLoad(attempt + 1), delay);
          } else {
            console.error('Could not connect to Vite dev server after', retries, 'attempts');
            console.error('Make sure to run "npm run dev" in another terminal');
            mainWindow.loadURL(`data:text/html,
              <html>
                <body style="font-family: Arial, sans-serif; padding: 20px;">
                  <h1>Development Server Not Running</h1>
                  <p>Vite dev server is not running at ${devUrl}</p>
                  <p>Please run in another terminal:</p>
                  <pre style="background: #f0f0f0; padding: 10px;">npm run dev</pre>
                  <p>Or run the production build:</p>
                  <pre style="background: #f0f0f0; padding: 10px;">npm run build && npm run electron:dev</pre>
                </body>
              </html>
            `);
          }
        });
      };

      attemptLoad();
    };

    // Start trying to connect
    tryLoadDevServer();

  } else {
    // PRODUCTION MODE
    console.log('=== PRODUCTION MODE ===');
    console.log('Resources path:', process.resourcesPath);
    console.log('App path:', app.getAppPath());

    // Simplified production loading - just try the most common paths
    const indexPath = path.join(__dirname, '..', 'dist', 'index.html');

    if (fs.existsSync(indexPath)) {
      console.log('✓ Found index.html at:', indexPath);
      mainWindow.loadFile(indexPath);
      console.log('✓ Successfully loaded production build');
    } else {
      // Fallback: Check resources path
      const altPath = path.join(process.resourcesPath, 'dist', 'index.html');
      if (fs.existsSync(altPath)) {
        console.log('✓ Found index.html at:', altPath);
        mainWindow.loadFile(altPath);
        console.log('✓ Successfully loaded production build');
      } else {
        console.error('Could not load production build');
        mainWindow.loadURL(`data:text/html,
          <html>
            <body style="font-family: Arial, sans-serif; padding: 20px;">
              <h1>Production Build Not Found</h1>
              <p>Could not find the built React application.</p>
              <p>Please run:</p>
              <pre style="background: #f0f0f0; padding: 10px;">npm run build</pre>
              <p>to build the application first.</p>
            </body>
          </html>
        `);
      }
    }
  }

  // Show window when ready
  mainWindow.once('ready-to-show', () => {
    console.log('Window is ready to show');
    mainWindow.show();
  });

  // Debug logging
  mainWindow.webContents.on('did-finish-load', () => {
    console.log('Page finished loading');
  });

  mainWindow.webContents.on('did-fail-load', (event, errorCode, errorDescription) => {
    console.error('Page failed to load:', errorCode, errorDescription);
  });
}

app.whenReady().then(() => {
  console.log('=== ELECTRON APP STARTING ===');
  console.log('App path:', app.getAppPath());
  console.log('User data path:', app.getPath('userData'));
  console.log('Resources path:', process.resourcesPath);
  console.log('Is packaged:', app.isPackaged);
  console.log('=============================');

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

      // Compiler executable path
      const exePath = isDev
        ? path.join(__dirname, '../main.exe')
        : path.join(process.resourcesPath, 'main.exe');

      console.log('Compiler executable path:', exePath);
      console.log('File exists:', fs.existsSync(exePath));

      // Folder to store input/output files
      const tempDir = isDev ? path.join(__dirname, '..') : app.getPath('userData');

      // File paths
      const inputPath = path.join(tempDir, 'input.txt');
      const outputPrintPath = path.join(tempDir, 'output_print.txt');
      const outputAssemblyPath = path.join(tempDir, 'output_assembly.txt');
      const outputMachineAssemblyPath = path.join(tempDir, 'output_machine_assembly.txt');
      const outputMachineBinaryPath = path.join(tempDir, 'output_machine_bin.txt');
      const outputMachineHexPath = path.join(tempDir, 'output_machine_hex.txt');

      // Clean up previous output files before new compilation
      const outputFiles = [
        outputPrintPath,
        outputAssemblyPath,
        outputMachineAssemblyPath,
        outputMachineBinaryPath,
        outputMachineHexPath
      ];

      outputFiles.forEach(filePath => {
        try {
          if (fs.existsSync(filePath)) {
            fs.unlinkSync(filePath);
            console.log(`Cleaned up: ${filePath}`);
          }
        } catch (err) {
          console.warn(`Could not delete ${filePath}:`, err.message);
        }
      });

      // Write source code
      fs.writeFileSync(inputPath, sourceCode, 'utf-8');

      // Run compiler
      const compilerProcess = spawn(exePath, [], {
        cwd: tempDir,
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
            machine: {
              assembly: safeRead(outputMachineAssemblyPath),
              binary: safeRead(outputMachineBinaryPath),
              hex: safeRead(outputMachineHexPath),
            },
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