import { useState, useEffect } from 'react';
import { CodeEditor } from './components/CodeEditor';
import { Toolbar } from './components/Toolbar';
import { WindowControls } from './components/WindowControls';
import { Play, Code2, Sun, Moon } from 'lucide-react';

export default function App() {
  // Load persisted state from localStorage
  const [sourceCode, setSourceCode] = useState(() => localStorage.getItem('sourceCode') || '');
  const [output, setOutput] = useState(() => localStorage.getItem('output') || '');
  const [targetCode, setTargetCode] = useState(() => localStorage.getItem('targetCode') || '');
  const [machineCode, setMachineCode] = useState(() => localStorage.getItem('machineCode') || '');
  const [isRunning, setIsRunning] = useState(false);

  const [theme, setTheme] = useState<'light' | 'dark'>(() => {
    const savedTheme = localStorage.getItem('theme') as 'light' | 'dark' | null;
    return savedTheme || 'dark';
  });

  useEffect(() => {
    const root = document.documentElement;
    if (theme === 'dark') root.classList.add('dark');
    else root.classList.remove('dark');
    localStorage.setItem('theme', theme);
  }, [theme]);

  useEffect(() => {
    localStorage.setItem('sourceCode', sourceCode);
  }, [sourceCode]);

  useEffect(() => {
    localStorage.setItem('output', output);
  }, [output]);

  useEffect(() => {
    localStorage.setItem('targetCode', targetCode);
  }, [targetCode]);

  useEffect(() => {
    localStorage.setItem('machineCode', machineCode);
  }, [machineCode]);

  const toggleTheme = () => setTheme(prev => (prev === 'dark' ? 'light' : 'dark'));

  const handleRun = async () => {
    if (!sourceCode.trim()) {
      setOutput('Error: Source code is empty.');
      return;
    }

    setIsRunning(true);
    setOutput('');
    setTargetCode('');
    setMachineCode('');

    if (window.electronAPI) {
      try {
        const result = await window.electronAPI.runCompiler(sourceCode);
        setOutput(result.outputs?.print ?? result.stdout ?? 'No output generated.');
        setTargetCode(result.outputs?.assembly ?? '');
        setMachineCode(result.outputs?.machine ?? '');

      } catch (error: any) {
        setOutput(`Error: ${error.message || 'Failed to run compiler'}`);
      } finally {
        setIsRunning(false);
      }
    } else {
      setOutput('Note: Electron API not available in development.');
      setIsRunning(false);
    }
  };

  const handleClearOutput = () => {
    setOutput('');
    setTargetCode('');
    setMachineCode('');
  };

  const handleClearInput = () => {
    setSourceCode('');
  }

  const handleImport = () => {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.txt,.bs';
    input.onchange = (e: any) => {
      const file = e.target.files[0];
      if (!file) return;
      const reader = new FileReader();
      reader.onload = () => setSourceCode(reader.result as string);
      reader.readAsText(file);
    };
    input.click();
  };

  const handleExport = () => {
    if (!targetCode) {
      alert('Nothing to export!');
      return;
    }

    if (targetCode.includes("No assembly generated due to parse errors.")) {
      alert('Nothing to export!');
      return;
    }


    const blob = new Blob([targetCode], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);

    const a = document.createElement('a');
    a.href = url;
    a.download = 'assembly.txt';
    a.click();

    URL.revokeObjectURL(url);
  };


  return (
    <div className={`min-h-screen transition-colors ${theme === 'dark'
      ? 'bg-linear-to-br from-gray-950 via-gray-900 to-black text-gray-100'
      : 'bg-linear-to-br from-gray-50 via-white to-gray-100 text-gray-900'}`}>

      {/* Header */}
      <header
        className={`border-b backdrop-blur-sm transition-colors ${theme === 'dark'
          ? 'border-gray-800 bg-black/50'
          : 'border-gray-300/70 bg-gray-200'
          }`}
        style={{ WebkitAppRegion: 'drag' } as React.CSSProperties}
      >
        <div className="flex items-center justify-between px-6 py-4">
          <div className="flex items-center gap-3">
            <div className="flex items-center justify-center w-10 h-10 rounded-lg bg-linear-to-br from-gray-800 to-gray-900 shadow-md">
              <Code2 className="w-6 h-6 text-white" />
            </div>
            <div>
              <h1 className={`text-xl font-semibold ${theme === 'dark' ? 'text-white' : 'text-gray-900'}`}>BaiScript IDE</h1>
              <p className={`${theme === 'dark' ? 'text-gray-400' : 'text-gray-600'} text-sm`}>Multi-stage Compiler Environment</p>
            </div>
          </div>

          {/* Header Buttons */}
          <div className="flex items-center gap-3" style={{ WebkitAppRegion: 'no-drag' } as React.CSSProperties}>
            <button type="button" onClick={handleClearInput} className={`px-4 py-2 rounded-lg border transition-all ${theme === 'dark'
              ? 'border-gray-700 hover:border-gray-600 hover:bg-gray-800/50'
              : 'border-gray-300/70 hover:border-gray-400 hover:bg-gray-100'}`}>
              Clear Input
            </button>

            <button type="button" onClick={handleClearOutput} className={`px-4 py-2 rounded-lg border transition-all ${theme === 'dark'
              ? 'border-gray-700 hover:border-gray-600 hover:bg-gray-800/50'
              : 'border-gray-300/70 hover:border-gray-400 hover:bg-gray-100'}`}>
              Clear Output
            </button>

            <button type="button" onClick={handleRun} disabled={isRunning} className={`flex items-center gap-2 px-6 py-2 rounded-lg transition-all disabled:opacity-50 disabled:cursor-not-allowed ${theme === 'dark'
              ? 'bg-gradient-to-r from-gray-700 to-gray-800 hover:from-gray-600 hover:to-gray-700 text-white shadow-md hover:shadow-lg'
              : 'bg-gradient-to-r from-slate-700 to-slate-900 hover:from-slate-800 hover:to-black text-white shadow-md hover:shadow-lg border border-gray-300'}`}>
              <Play className="w-4 h-4" /> {isRunning ? 'Running...' : 'Run'}
            </button>

            <button type="button" onClick={toggleTheme} className={`p-2 rounded-lg border transition-all ${theme === 'dark'
              ? 'border-gray-700 hover:border-gray-600 hover:bg-gray-800/50'
              : 'border-gray-300/70 hover:border-gray-400 hover:bg-gray-100'}`}
              title={theme === 'dark' ? 'Switch to light mode' : 'Switch to dark mode'}>
              {theme === 'dark' ? <Sun className="w-5 h-5" /> : <Moon className="w-5 h-5" />}
            </button>

            <div style={{ WebkitAppRegion: 'no-drag' } as React.CSSProperties}>
              <WindowControls />
            </div>
          </div>
        </div>
      </header>

      <Toolbar
        onImport={handleImport}
        onExport={handleExport}
        theme={theme}
      />

      {/* Main Content Grid */}
      <main className="grid grid-cols-2 grid-rows-2 gap-4 p-4 h-[calc(100vh-140px)]">
        <CodeEditor title="Source Code" subtitle="Input" value={sourceCode} onChange={setSourceCode} editable language="BaiScript" theme={theme} />
        <CodeEditor title="Output" subtitle="Execution Result" value={output} readOnly placeholder="Output will appear here..." theme={theme} />
        <CodeEditor title="Target Code" subtitle="Assembly Output" value={targetCode} readOnly placeholder="Compiled intermediate code..." language="MIPS64" theme={theme} />
        <CodeEditor title="Machine Code" subtitle="Low Level Representation" value={machineCode} readOnly placeholder="Generated machine code..." language="Binary/Hex" theme={theme} />
      </main>
    </div>
  );
}
