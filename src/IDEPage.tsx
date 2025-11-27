import { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { CodeEditor } from './components/CodeEditor';
import { Toolbar } from './components/Toolbar';
import { WindowControls } from './components/WindowControls';
import { Play, Code2, Sun, Moon } from 'lucide-react';

// Define the machine code type
interface MachineCodeValue {
    assembly: string;
    binary: string;
    hex: string;
}

export default function IDEPage() {
    const navigate = useNavigate();

    // Load persisted state from localStorage
    const [sourceCode, setSourceCode] = useState(() => localStorage.getItem('sourceCode') || '');
    const [output, setOutput] = useState(() => localStorage.getItem('output') || '');
    const [targetCode, setTargetCode] = useState(() => localStorage.getItem('targetCode') || '');

    // Fix: Properly type machineCode state - can be string (for errors) or object (for success)
    const [machineCode, setMachineCode] = useState<MachineCodeValue | string>(() => {
        const saved = localStorage.getItem('machineCode');
        if (saved) {
            try {
                // Try to parse as object, fall back to string
                const parsed = JSON.parse(saved);
                if (parsed && typeof parsed === 'object' && ('assembly' in parsed || 'binary' in parsed || 'hex' in parsed)) {
                    return parsed;
                }
                return saved;
            } catch {
                return saved;
            }
        }
        return '';
    });

    const [isRunning, setIsRunning] = useState(false);
    const [theme, setTheme] = useState<'light' | 'dark'>(() => {
        const savedTheme = localStorage.getItem('theme') as 'light' | 'dark' | null;
        return savedTheme || 'dark';
    });

    const [notification, setNotification] = useState('');

    useEffect(() => {
        const root = document.documentElement;
        if (theme === 'dark') root.classList.add('dark');
        else root.classList.remove('dark');
        localStorage.setItem('theme', theme);
    }, [theme]);

    useEffect(() => { localStorage.setItem('sourceCode', sourceCode); }, [sourceCode]);
    useEffect(() => { localStorage.setItem('output', output); }, [output]);
    useEffect(() => { localStorage.setItem('targetCode', targetCode); }, [targetCode]);

    // Fix: Properly serialize machineCode for localStorage
    useEffect(() => {
        if (typeof machineCode === 'string') {
            localStorage.setItem('machineCode', machineCode);
        } else {
            localStorage.setItem('machineCode', JSON.stringify(machineCode));
        }
    }, [machineCode]);

    // Helper function to check if machine output exists
    const hasMachineOutput = (machine: any): boolean => {
        if (!machine) return false;
        if (typeof machine === 'string') return machine.trim().length > 0;

        return (
            (machine.assembly && machine.assembly.trim().length > 0) ||
            (machine.binary && machine.binary.trim().length > 0) ||
            (machine.hex && machine.hex.trim().length > 0)
        );
    };

    const toggleTheme = () => setTheme(prev => prev === 'dark' ? 'light' : 'dark');

    const handleRun = async () => {
        if (!sourceCode.trim()) {
            setOutput('Error: Source code is empty.');
            return;
        }

        setIsRunning(true);
        setOutput('');
        setTargetCode('');
        setMachineCode(''); // Reset to empty string

        if (window.electronAPI) {
            try {
                const result = await window.electronAPI.runCompiler(sourceCode);

                // Check if compilation was successful
                const isSuccess = result.success && result.exitCode === 0;

                if (isSuccess) {
                    // SUCCESS: Use structured machine code with tabs
                    // For successful compilation, use the print output or stdout
                    const printOutput = result.outputs?.print || result.stdout || 'No output generated.';
                    setOutput(printOutput);
                    setTargetCode(result.outputs?.assembly || '');

                    if (result.outputs?.machine && hasMachineOutput(result.outputs.machine)) {
                        setMachineCode(result.outputs.machine);
                    } else {
                        // Fallback if machine code is not in expected format
                        setMachineCode({
                            assembly: result.outputs?.assembly || 'No assembly generated',
                            binary: 'No binary output generated',
                            hex: 'No hex output generated'
                        });
                    }
                } else {
                    // COMPILATION ERROR: Use output_print.txt content for the Output panel
                    // This is the key change - use the print output for errors
                    const errorOutput = result.outputs?.print ||
                        result.stderr ||
                        result.stdout ||
                        'Compilation failed with no error message.';

                    setOutput(errorOutput);

                    // For errors, show error messages in machine code panel as plain text
                    if (result.outputs?.print && result.outputs.print.trim().length > 0) {
                        // Use the print output as machine code error display
                        setMachineCode(result.outputs.print);
                    } else if (result.stderr && result.stderr.trim().length > 0) {
                        // Use stderr as machine code error display
                        setMachineCode(result.stderr);
                    } else {
                        // Generic error message
                        setMachineCode('Compilation failed - no machine code generated');
                    }

                    // For target code, show error or partial output
                    if (result.outputs?.assembly && result.outputs.assembly.trim().length > 0) {
                        setTargetCode(result.outputs.assembly + '\n\n--- Partial output due to compilation errors ---');
                    } else {
                        setTargetCode('No assembly generated due to compilation errors');
                    }
                }
            } catch (error: any) {
                setOutput(`Error: ${error.message || 'Failed to run compiler'}`);
                setMachineCode('Compiler execution failed - unable to run compiler process');
                setTargetCode('Compiler execution failed');
            } finally {
                setIsRunning(false);
            }
        } else {
            setOutput('Note: Electron API not available in development.');
            setMachineCode('Electron API not available in development mode');
            setTargetCode('Development mode - no compilation');
            setIsRunning(false);
        }
    };

    const handleClearOutput = () => {
        setOutput('');
        setTargetCode('');
        setMachineCode(''); // Reset to empty string
    };

    const handleClearInput = () => setSourceCode('');

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
        // Fix: Check if we have valid target code to export
        const exportCode = typeof targetCode === 'string' ? targetCode : '';

        if (!exportCode ||
            exportCode.includes("No assembly generated") ||
            exportCode.includes("Compiler execution failed") ||
            exportCode.includes("Development mode") ||
            exportCode.includes("Partial output due to compilation errors")) {
            setNotification('Nothing to export!');
            setTimeout(() => setNotification(''), 3000);
            return;
        }

        try {
            const blob = new Blob([exportCode], { type: 'text/plain' });
            const url = URL.createObjectURL(blob);

            const a = document.createElement('a');
            a.href = url;
            a.download = 'assembly.txt';
            a.click();

            URL.revokeObjectURL(url);
        } catch (error: any) {
            setNotification('Export failed!');
            setTimeout(() => setNotification(''), 3000);
            console.error(error);
        }
    };

    return (
        <div className={`min-h-screen transition-colors ${theme === 'dark'
            ? 'bg-linear-to-br from-gray-950 via-gray-900 to-black text-gray-100'
            : 'bg-linear-to-br from-gray-50 via-white to-gray-100 text-gray-900'}`}>

            {/* Notification Toast */}
            {notification && (
                <div className={`fixed top-0 left-1/2 transform -translate-x-1/2 mt-4 px-6 py-4 rounded-xl shadow-lg z-50 text-lg font-semibold transition-all duration-300 ease-out
          ${theme === 'dark' ? 'bg-gray-800 text-white' : 'bg-gray-200 text-gray-900'}
          ${notification ? 'translate-y-0 opacity-100' : '-translate-y-12 opacity-0'}`}>
                    {notification}
                </div>
            )}

            {/* Header */}
            <header className={`border-b backdrop-blur-sm transition-colors ${theme === 'dark'
                ? 'border-gray-800 bg-black/50' : 'border-gray-300/70 bg-gray-200'}`}
                style={{ WebkitAppRegion: 'drag' } as React.CSSProperties}>
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
                        <button type="button" onClick={handleClearInput} className={`px-4 py-2 rounded-lg transition-all ${theme === 'dark' ? 'border border-gray-700 hover:border-gray-600 hover:bg-gray-800/50' : 'border border-gray-400 hover:border-gray-500 hover:bg-gray-100'}`}>Clear Input</button>
                        <button type="button" onClick={handleClearOutput} className={`px-4 py-2 rounded-lg transition-all ${theme === 'dark' ? 'border border-gray-700 hover:border-gray-600 hover:bg-gray-800/50' : 'border border-gray-400 hover:border-gray-500 hover:bg-gray-100'}`}>Clear Output</button>
                        <button type="button" onClick={() => navigate('/docs')} className={`px-4 py-2 rounded-lg transition-all ${theme === 'dark' ? 'border border-gray-700 hover:border-gray-600 hover:bg-gray-800/50' : 'border border-gray-400 hover:border-gray-500 hover:bg-gray-100'}`}>Docs</button>
                        <button type="button" onClick={handleRun} disabled={isRunning} className={`flex items-center gap-2 px-6 py-2 rounded-lg transition-all disabled:opacity-50 disabled:cursor-not-allowed ${theme === 'dark' ? 'bg-gradient-to-r from-gray-700 to-gray-800 hover:from-gray-600 hover:to-gray-700 text-white shadow-md hover:shadow-lg' : 'bg-gradient-to-r from-slate-700 to-slate-900 hover:from-slate-800 hover:to-black text-white shadow-md hover:shadow-lg border border-gray-300'}`}>
                            <Play className="w-4 h-4" /> {isRunning ? 'Running...' : 'Run'}
                        </button>
                        <button type="button" onClick={toggleTheme} className={`p-2 rounded-lg border transition-all ${theme === 'dark' ? 'border-gray-700 hover:border-gray-600 hover:bg-gray-800/50' : 'border border-gray-300/70 hover:border-gray-400 hover:bg-gray-100'}`} title={theme === 'dark' ? 'Switch to light mode' : 'Switch to dark mode'}>
                            {theme === 'dark' ? <Sun className="w-5 h-5" /> : <Moon className="w-5 h-5" />}
                        </button>
                        <div style={{ WebkitAppRegion: 'no-drag' } as React.CSSProperties}><WindowControls /></div>
                    </div>
                </div>
            </header>

            <Toolbar onImport={handleImport} onExport={handleExport} theme={theme} />

            {/* Main Content Grid */}
            <main className="grid grid-cols-2 grid-rows-2 gap-4 p-4 h-[calc(100vh-140px)]">
                <CodeEditor
                    title="Source Code"
                    subtitle="Input"
                    value={sourceCode}
                    onChange={setSourceCode}
                    editable
                    language="BaiScript"
                    theme={theme}
                />
                <CodeEditor
                    title="Output"
                    subtitle="Execution Result"
                    value={output}
                    readOnly
                    placeholder="Output will appear here..."
                    theme={theme}
                />
                <CodeEditor
                    title="Target Code"
                    subtitle="Assembly Output"
                    value={targetCode}
                    readOnly
                    placeholder="Compiled intermediate code..."
                    language="MIPS64"
                    theme={theme}
                />
                <CodeEditor
                    title="Machine Code"
                    subtitle="Low Level Representation"
                    value={machineCode}
                    readOnly
                    placeholder="Generated machine code..."
                    language="Binary/Hex"
                    theme={theme}
                />
            </main>
        </div>
    );
}