import { Terminal, Code, Cpu, FileCode } from 'lucide-react';

interface CodeEditorProps {
  title: string;
  subtitle: string;
  value: string;
  onChange?: (value: string) => void;
  readOnly?: boolean;
  editable?: boolean;
  placeholder?: string;
  language?: string;
  theme?: 'light' | 'dark';
}

export function CodeEditor({
  title,
  subtitle,
  value,
  onChange,
  readOnly = false,
  editable = false,
  placeholder = '',
  language = 'text',
  theme = 'dark'
}: CodeEditorProps) {
  const getIcon = () => {
    if (title.includes('Source')) return <FileCode className="w-4 h-4" />;
    if (title.includes('Output')) return <Terminal className="w-4 h-4" />;
    if (title.includes('Target')) return <Code className="w-4 h-4" />;
    if (title.includes('Machine')) return <Cpu className="w-4 h-4" />;
    return <Code className="w-4 h-4" />;
  };

  const isDark = theme === 'dark';

  const getAccentColor = () => {
    if (isDark) {
      if (title.includes('Source')) return 'from-gray-900/40 to-gray-950/20';
      if (title.includes('Output')) return 'from-gray-900/30 to-gray-950/20';
      if (title.includes('Target')) return 'from-gray-900/35 to-gray-950/20';
      if (title.includes('Machine')) return 'from-gray-900/45 to-gray-950/20';
      return 'from-gray-900/20 to-gray-950/10';
    } else {
      if (title.includes('Source')) return 'from-gray-100/80 to-gray-50/40';
      if (title.includes('Output')) return 'from-gray-100/70 to-gray-50/40';
      if (title.includes('Target')) return 'from-gray-100/75 to-gray-50/40';
      if (title.includes('Machine')) return 'from-gray-100/85 to-gray-50/40';
      return 'from-gray-100/60 to-gray-50/30';
    }
  };

  const getBorderColor = () => {
    return isDark ? 'border-gray-700/50' : 'border-gray-300/50';
  };

  return (
    <div className={`flex flex-col rounded-xl border ${getBorderColor()} bg-gradient-to-br ${getAccentColor()} backdrop-blur-sm overflow-hidden transition-colors`}>
      {/* Header */}
      <div className={`flex items-center justify-between px-4 py-3 border-b transition-colors ${isDark
        ? 'border-gray-800/50 bg-black/30'
        : 'border-gray-200/50 bg-white/50'
        }`}>
        <div className="flex items-center gap-2">
          {getIcon()}
          <div>
            <h3 className="text-sm">{title}</h3>
            <p className={`text-xs transition-colors ${isDark
              ? 'text-gray-400'
              : 'text-gray-600'
              }`}>
              {subtitle}
            </p>
          </div>
        </div>
        <div className="flex items-center gap-2">
          <span className={`text-xs transition-colors ${isDark
            ? 'text-gray-500'
            : 'text-gray-600'
            }`}>
            {language}
          </span>
          <div className="flex gap-1.5">
            <div className={`w-3 h-3 rounded-full transition-colors ${isDark
              ? 'bg-gray-600/50'
              : 'bg-gray-400/50'
              }`}></div>
            <div className={`w-3 h-3 rounded-full transition-colors ${isDark
              ? 'bg-gray-600/50'
              : 'bg-gray-400/50'
              }`}></div>
            <div className={`w-3 h-3 rounded-full transition-colors ${isDark
              ? 'bg-gray-600/50'
              : 'bg-gray-400/50'
              }`}></div>
          </div>
        </div>
      </div>

      {/* Editor Area */}
      <div className="flex-1 relative">
        <textarea
          value={value}
          onChange={(e) => onChange?.(e.target.value)}
          readOnly={readOnly}
          placeholder={placeholder}
          spellCheck={false}
          className={`w-full h-full p-4 bg-transparent text-sm font-mono resize-none focus:outline-none transition-colors ${isDark
            ? 'text-gray-100 placeholder:text-gray-600'
            : 'text-gray-900 placeholder:text-gray-500'
            }`}
          style={{
            lineHeight: '1.6',
            tabSize: 2
          }}
        />

        {/* Line numbers overlay effect */}
        <div className={`absolute top-0 left-0 w-12 h-full bg-gradient-to-r transition-colors pointer-events-none ${isDark
          ? 'from-black/20 to-transparent'
          : 'from-white/40 to-transparent'
          }`}></div>
      </div>

      {/* Footer */}
      <div className={`flex items-center justify-between px-4 py-2 border-t transition-colors ${isDark
        ? 'border-gray-800/50 bg-black/30'
        : 'border-gray-200/50 bg-white/50'
        }`}>
        <span className={`text-xs transition-colors ${isDark
          ? 'text-gray-500'
          : 'text-gray-600'
          }`}>
          {value.split('\n').length} lines
        </span>
        <span className={`text-xs transition-colors ${isDark
          ? 'text-gray-500'
          : 'text-gray-600'
          }`}>
          {value.length} characters
        </span>
      </div>
    </div>
  );
}