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
  const isDark = theme === 'dark';

  const getIcon = () => {
    if (title.includes('Source')) return <FileCode className="w-4 h-4" />;
    if (title.includes('Output')) return <Terminal className="w-4 h-4" />;
    if (title.includes('Target')) return <Code className="w-4 h-4" />;
    if (title.includes('Machine')) return <Cpu className="w-4 h-4" />;
    return <Code className="w-4 h-4" />;
  };

  const getEditorBg = () => {
    if (isDark) {
      return title.includes('Source')
        ? 'bg-gradient-to-br from-gray-900/40 to-gray-950/20'
        : 'bg-gradient-to-br from-gray-900/30 to-gray-950/15';
    } else {
      return 'bg-white';
    }
  };

  const getTextColor = () => (isDark ? 'text-white' : 'text-black');
  const getPlaceholderColor = () => (isDark ? 'placeholder:text-gray-500' : 'placeholder:text-gray-400');

  const getBorderColor = () => (isDark ? 'border-gray-700/50' : 'border-gray-300/50');

  const get_bg_color = () => (isDark ? '' : 'bg-white');

  // Calculate line numbers if the title is "Source"
  const lines = title.includes('Source') ? value.split('\n') : [];

  const handleScroll = (e: React.SyntheticEvent) => {
    const textArea = e.target as HTMLTextAreaElement;
    const lineNumbers = textArea.previousSibling as HTMLElement;
    if (lineNumbers) {
      lineNumbers.scrollTop = textArea.scrollTop;
    }
  };

  return (
    <div className={`flex flex-col rounded-xl border ${getBorderColor()} backdrop-blur-sm overflow-hidden transition-colors
                    shadow-md hover:shadow-lg ${get_bg_color()}`}>

      {/* Header */}
      <div className={`flex items-center justify-between px-4 py-3 transition-colors
        ${isDark ? 'border-gray-800/50 bg-black/30' : 'border-gray-300/70 bg-gray-200'}`}>
        <div className="flex items-center gap-2">
          {getIcon()}
          <div>
            <h3 className="text-sm">{title}</h3>
            <p className={`text-xs transition-colors ${isDark ? 'text-gray-400' : 'text-gray-600'}`}>
              {subtitle}
            </p>
          </div>
        </div>
        <div className="flex items-center gap-2">
          <span className={`text-xs transition-colors ${isDark ? 'text-gray-500' : 'text-gray-600'}`}>
            {language}
          </span>
          <div className="flex gap-1.5">
            <div className={`w-3 h-3 rounded-full transition-colors ${isDark ? 'bg-gray-600/50' : 'bg-gray-400/50'}`}></div>
            <div className={`w-3 h-3 rounded-full transition-colors ${isDark ? 'bg-gray-600/50' : 'bg-gray-400/50'}`}></div>
            <div className={`w-3 h-3 rounded-full transition-colors ${isDark ? 'bg-gray-600/50' : 'bg-gray-400/50'}`}></div>
          </div>
        </div>
      </div>

      {/* Editor Area */}
      <div className="flex-1 overflow-hidden relative">
        {title.includes('Source') && (
          <div className="absolute top-0 left-0 bottom-0 overflow-y-scroll no-scrollbar pr-2 w-12 text-center text-[14px] leading-[1.6] font-mono .no-scrollbar overflow-hidden">
            {lines.map((_, index) => (
              <div
                key={index}
                className={`${isDark ? 'text-gray-500' : 'text-gray-400'}`}
              >
                {index + 1}
              </div>
            ))}
          </div>

        )}

        <textarea
          value={value}
          onChange={(e) => onChange?.(e.target.value)}
          readOnly={readOnly}
          placeholder={placeholder}
          spellCheck={false}
          onScroll={handleScroll}
          className={`
          w-full h-full pl-9 pr-4 text-sm font-mono resize-none
          overflow-y-scroll box-content
          focus:outline-none transition-colors
          ${getEditorBg()} ${getTextColor()} ${getPlaceholderColor()}
          border-none shadow-inner rounded-none
        `}
          style={{ lineHeight: '1.6', tabSize: 2 }}
        />
      </div>

      {/* Footer */}
      <div className={`flex items-center justify-between px-4 py-2 transition-colors
        ${isDark ? 'border-gray-800/50 bg-black/30' : 'border-gray-300/70 bg-gray-200'}`}>
        <span className={`text-xs transition-colors ${isDark ? 'text-gray-500' : 'text-gray-600'}`}>
          {lines.length} lines
        </span>
        <span className={`text-xs transition-colors ${isDark ? 'text-gray-500' : 'text-gray-600'}`}>
          {value.length} characters
        </span>
      </div>
    </div>
  );
}
