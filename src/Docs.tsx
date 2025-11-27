import { useState } from 'react';
import { Navigation } from './components/Navigations';
import { DocumentationContent } from './components/DocumentationContent';
import { Moon, Sun } from 'lucide-react';
import { useNavigate } from 'react-router-dom';
import { WindowControls } from './components/WindowControls';

export default function Docs() {
    const navigate = useNavigate();
    const [isDarkMode, setIsDarkMode] = useState(true);
    const [activeSection, setActiveSection] = useState('introduction');

    return (
        <div className={`min-h-screen transition-colors ${isDarkMode
            ? 'bg-linear-to-br from-gray-950 via-gray-900 to-black text-gray-100'
            : 'bg-linear-to-br from-gray-50 via-white to-gray-100 text-gray-900'}`}>

            {/* Header */}
            <header className={`sticky top-0 z-50 border-b backdrop-blur-sm transition-colors ${isDarkMode
                ? 'border-gray-800 bg-black/50'
                : 'border-gray-300/70 bg-gray-200'}`}>
                <div className="flex items-center justify-between px-6 py-4">
                    {/* Left: Logo + Title */}
                    <div className="flex items-center gap-3">
                        <div className="w-10 h-10 bg-linear-to-br from-gray-800 to-gray-900 rounded-lg flex items-center justify-center shadow-md">
                            <span className="text-white text-xl">{'</>'}</span>
                        </div>
                        <div>
                            <h1 className={`text-gray-900 dark:text-white text-lg font-semibold`}>BaiScript Documentation</h1>
                            <p className={`text-sm ${isDarkMode ? 'text-gray-400' : 'text-gray-600'}`}>Multi-stage Compiler Environment</p>
                        </div>
                    </div>

                    {/* Right: Theme Toggle + Back Button */}
                    <div className="flex items-center gap-3">
                        {/* Theme Toggle */}
                        <button
                            onClick={() => setIsDarkMode(!isDarkMode)}
                            className={`p-2 rounded-lg border transition-all ${isDarkMode ? 'border-gray-700 hover:border-gray-600 hover:bg-gray-800/50' : 'border border-gray-300/70 hover:border-gray-400 hover:bg-gray-100'}`}
                            aria-label="Toggle theme"
                        >
                            {isDarkMode ? <Sun className="w-5 h-5" /> : <Moon className="w-5 h-5" />}
                        </button>

                        {/* Back to IDE */}
                        <button
                            onClick={() => navigate('/')}
                            className={`px-4 py-2 rounded-lg border transition-all ${isDarkMode ? 'border-gray-700 hover:border-gray-600 hover:bg-gray-800/50 text-gray-100' : 'border border-gray-300 hover:border-gray-400 hover:bg-gray-100 text-gray-900'}`}
                        >
                            ← Back to IDE
                        </button>

                        <div style={{ WebkitAppRegion: 'no-drag' } as React.CSSProperties}>
                            <WindowControls />
                        </div>

                    </div>
                </div>
            </header>

            {/* Main Content */}
            <div className="flex">
                <Navigation
                    activeSection={activeSection}
                    setActiveSection={setActiveSection}
                    isDarkMode={isDarkMode}
                />
                <DocumentationContent
                    activeSection={activeSection}
                    isDarkMode={isDarkMode}
                />
            </div>
        </div>
    );
}
