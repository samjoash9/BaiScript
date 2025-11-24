import { useState, useEffect } from 'react';
import { CodeEditor } from './components/CodeEditor';
import { Toolbar } from './components/Toolbar';
import { WindowControls } from './components/WindowControls';
import { Play, Code2, Sun, Moon } from 'lucide-react';

export default function App() {
  const [sourceCode, setSourceCode] = useState(``);

  const [output, setOutput] = useState('');
  const [targetCode, setTargetCode] = useState('');
  const [machineCode, setMachineCode] = useState('');
  const [isRunning, setIsRunning] = useState(false);
  const [theme, setTheme] = useState<'light' | 'dark'>(() => {
    const savedTheme = localStorage.getItem('theme') as 'light' | 'dark' | null;
    return savedTheme || 'dark';
  });

  useEffect(() => {
    const root = document.documentElement;
    if (theme === 'dark') {
      root.classList.add('dark');
    } else {
      root.classList.remove('dark');
    }
    localStorage.setItem('theme', theme);
  }, [theme]);

  const toggleTheme = () => {
    setTheme(prev => prev === 'dark' ? 'light' : 'dark');
  };

  const handleRun = async () => {
    if (!sourceCode.trim()) {
      setOutput('Error: Source code is empty. Please write some code first.');
      return;
    }

    setIsRunning(true);
    setOutput('');
    setTargetCode('');
    setMachineCode('');

    // Check if running in Electron
    if (window.electronAPI) {
      try {
        const result = await window.electronAPI.runCompiler(sourceCode);

        if (result.success) {
          const printOutput = result.outputs?.print ?? result.stdout ?? '';
          setOutput(printOutput || 'No output generated.');
          setTargetCode(result.outputs?.assembly ?? '');
          setMachineCode(result.outputs?.machine ?? '');
        } else {
          setOutput(`Error: Compilation failed\nExit code: ${result.exitCode}\n${result.stderr || result.error || 'Unknown error'}`);
        }
      } catch (error: any) {
        setOutput(`Error: ${error.message || 'Failed to run compiler'}`);
      } finally {
        setIsRunning(false);
      }
    } else {
      // Fallback for development (not in Electron)
      setOutput('Note: Running in development mode. Electron API not available.\nPlease run with "npm run electron:dev" to use the compiler.');
      setIsRunning(false);
    }
  };

  const handleClear = () => {
    setOutput('');
    setTargetCode('');
    setMachineCode('');
  };

  const handleImport = () => {
    const input = document.createElement("input");
    input.type = "file";
    input.accept = ".txt,.bs"; // Accept .txt or your custom extension
    input.onchange = (e: any) => {
      const file = e.target.files[0];
      if (!file) return;

      const reader = new FileReader();
      reader.onload = () => {
        setSourceCode(reader.result as string); // Load into the editor
      };
      reader.readAsText(file);
    };

    input.click();
  };

  return (
    <div className={`min-h-screen transition-colors ${theme === 'dark'
      ? 'bg-gradient-to-br from-gray-950 via-gray-900 to-black text-gray-100'
      : 'bg-gradient-to-br from-gray-50 via-white to-gray-100 text-gray-900'
      }`}>
      {/* Header */}
      <header
        className={`border-b backdrop-blur-sm transition-colors ${theme === 'dark'
          ? 'border-gray-800 bg-black/50'
          : 'border-gray-200 bg-white/80'
          }`}
        style={{ WebkitAppRegion: 'drag' } as React.CSSProperties}
      >
        <div className="flex items-center justify-between px-6 py-4">
          <div className="flex items-center gap-3">
            <div className="flex items-center justify-center w-10 h-10 rounded-lg 
                  bg-gradient-to-br from-gray-800 to-gray-900 shadow-md">
              <Code2 className="w-6 h-6 text-white" />  {/* stays white */}
            </div>
            <div>
              <h1 className="text-xl">BaiScript IDE</h1>
              <p className="text-sm text-gray-400">
                Multi-stage Compiler Environment
              </p>
            </div>
          </div>


          <div className="flex items-center gap-3" style={{ WebkitAppRegion: 'no-drag' } as React.CSSProperties}>
            <button
              onClick={handleClear}
              className={`px-4 py-2 rounded-lg border transition-all ${theme === 'dark'
                ? 'border-gray-700 hover:border-gray-600 hover:bg-gray-800/50'
                : 'border-gray-300 hover:border-gray-400 hover:bg-gray-100'
                }`}
            >
              Clear Output
            </button>
            <button
              onClick={handleRun}
              disabled={isRunning}
              className={`flex items-center gap-2 px-6 py-2 rounded-lg transition-all disabled:opacity-50 disabled:cursor-not-allowed
    ${theme === 'dark'
                  ? 'bg-gradient-to-r from-gray-700 to-gray-800 hover:from-gray-600 hover:to-gray-700 text-white shadow-md hover:shadow-lg'
                  : 'bg-gradient-to-r from-slate-700 to-slate-900 hover:from-slate-800 hover:to-black text-white shadow-md hover:shadow-lg border border-gray-300'
                }`}
            >
              <Play className="w-4 h-4" />
              {isRunning ? 'Running...' : 'Run'}
            </button>

            <button
              onClick={toggleTheme}
              className={`p-2 rounded-lg border transition-all ${theme === 'dark'
                ? 'border-gray-700 hover:border-gray-600 hover:bg-gray-800/50'
                : 'border-gray-300 hover:border-gray-400 hover:bg-gray-100'
                }`}
              title={theme === 'dark' ? 'Switch to light mode' : 'Switch to dark mode'}
            >
              {theme === 'dark' ? (
                <Sun className="w-5 h-5" />
              ) : (
                <Moon className="w-5 h-5" />
              )}
            </button>
            <div style={{ WebkitAppRegion: 'no-drag' } as React.CSSProperties}>
              <WindowControls />
            </div>
          </div>
        </div>
      </header>

      <Toolbar onImport={handleImport} theme={theme} />


      {/* Main Content Grid */}
      <main className="grid grid-cols-2 grid-rows-2 gap-4 p-4 h-[calc(100vh-140px)]">
        {/* Source Code */}
        <CodeEditor
          title="Source Code"
          subtitle="Input"
          value={sourceCode}
          onChange={setSourceCode}
          editable={true}
          language="BaiScript"
          theme={theme}
        />

        {/* Output */}
        <CodeEditor
          title="Output"
          subtitle="Execution Result"
          value={output}
          readOnly={true}
          placeholder="Output will appear here after running..."
          theme={theme}
        />

        {/* Target Code */}
        <CodeEditor
          title="Target Code"
          subtitle="Assembly Output"
          value={targetCode}
          readOnly={true}
          placeholder="Compiled intermediate code will appear here..."
          language="MIPS64"
          theme={theme}
        />

        {/* Machine Code */}
        <CodeEditor
          title="Machine Code"
          subtitle="Low Level Representation"
          value={machineCode}
          readOnly={true}
          placeholder="Generated machine code will appear here..."
          language="Binary and Hexadecimal"
          theme={theme}
        />
      </main>
    </div>
  );
}