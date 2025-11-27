import { BookOpen, Code2, Variable, Equal, Calculator, ArrowUpDown, Terminal } from 'lucide-react';

interface NavigationProps {
    activeSection: string;
    setActiveSection: (section: string) => void;
    isDarkMode: boolean;
}

export function Navigation({ activeSection, setActiveSection, isDarkMode }: NavigationProps) {
    const sections = [
        { id: 'introduction', label: 'Introduction', icon: BookOpen },
        { id: 'datatypes', label: 'Datatypes', icon: Code2 },
        { id: 'variables', label: 'Variables and Declarations', icon: Variable },
        { id: 'assignments', label: 'Assignments', icon: Equal },
        { id: 'expressions', label: 'Expressions and Prefix/Postfix Operators', icon: Calculator },
        { id: 'io', label: 'Input/Output', icon: Terminal },
    ];

    return (
        <nav
            className={`sticky top-[73px] h-[calc(100vh-73px)] w-64 border-r transition-colors
            ${isDarkMode ? 'border-gray-800 bg-black/50' : 'border-gray-300 bg-gray-200'} overflow-y-auto`}
        >
            <div className="p-4">
                <h2 className={`text-sm mb-4 px-3 transition-colors ${isDarkMode ? 'text-gray-400' : 'text-gray-500'}`}>
                    Contents
                </h2>
                <ul className="space-y-1">
                    {sections.map((section) => {
                        const Icon = section.icon;
                        const isActive = activeSection === section.id;

                        return (
                            <li key={section.id}>
                                <button
                                    type="button"
                                    onClick={() => setActiveSection(section.id)}
                                    className={`w-full flex items-center gap-3 px-3 py-2 rounded-lg text-left transition-colors focus:outline-none
                                        ${isActive
                                            ? isDarkMode
                                                ? 'bg-gradient-to-r from-gray-700 to-gray-800 text-white'
                                                : 'bg-gray-100 text-gray-900'
                                            : isDarkMode
                                                ? 'text-gray-300 hover:bg-gray-900/20'
                                                : 'text-gray-700 hover:bg-gray-100'
                                        }`}
                                >
                                    <Icon
                                        className={`w-4 h-4 transition-colors ${isActive
                                            ? isDarkMode
                                                ? 'text-white'
                                                : 'text-gray-900'
                                            : isDarkMode
                                                ? 'text-gray-400'
                                                : 'text-gray-500'
                                            }`}
                                    />
                                    <span className="text-sm">{section.label}</span>
                                </button>
                            </li>
                        );
                    })}
                </ul>
            </div>
        </nav>
    );
}
