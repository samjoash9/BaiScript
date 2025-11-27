import { Terminal, Code, Cpu, FileCode } from 'lucide-react';
import { useState, useEffect } from 'react';

interface MachineCodeValue {
  assembly: string;
  binary: string;
  hex: string;
}

interface CodeEditorProps {
  title: string;
  subtitle: string;
  value: string | MachineCodeValue;
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
  theme = 'dark',
}: CodeEditorProps) {
  const isDark = theme === 'dark';
  const [editorText, setEditorText] = useState<string>('');

  // Sync editorText with value prop correctly
  useEffect(() => {
    if (typeof value === 'string') {
      setEditorText(value);
    } else if (title.includes('Machine')) {
      // For machine code, we'll handle it in the custom renderer
      setEditorText('');
    } else {
      // For Output/Target editors, pick a string representation
      setEditorText(JSON.stringify(value, null, 2));
    }
  }, [value, title]);

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

  const lines = title.includes('Source') && typeof value === 'string' ? value.split('\n') : [];

  const handleScroll = (e: React.SyntheticEvent) => {
    const textArea = e.target as HTMLTextAreaElement;
    const lineNumbers = textArea.previousSibling as HTMLElement;
    if (lineNumbers) lineNumbers.scrollTop = textArea.scrollTop;
  };

  // Custom rendering for machine code sections side by side
  const renderMachineCodeSections = () => {
    if (typeof value !== 'string' && title.includes('Machine')) {
      const machineValue = value as MachineCodeValue;

      return (
        <div className="flex-1 flex overflow-hidden">
          {/* Assembly Section - Medium width */}
          <div className="w-3/10 flex flex-col border-r border-gray-600/30">
            <div className={`px-4 py-2 font-semibold text-sm border-b border-gray-600/30 ${isDark ? 'bg-gray-800/50 text-gray-300' : 'bg-gray-100 text-gray-700'}`}>
              ASSEMBLY CODE
            </div>
            <textarea
              value={machineValue.assembly || 'No assembly generated'}
              readOnly
              spellCheck={false}
              className={`flex-1 w-full pl-4 pr-4 text-sm font-mono resize-none focus:outline-none transition-colors ${getEditorBg()} ${getTextColor()} ${getPlaceholderColor()} border-none`}
              style={{ lineHeight: '1.6', tabSize: 2 }}
            />
          </div>

          {/* Binary Section - Extra wide for long binary strings */}
          <div className="w-5/10 flex flex-col border-r border-gray-600/30">
            <div className={`px-4 py-2 font-semibold text-sm border-b border-gray-600/30 ${isDark ? 'bg-gray-800/50 text-gray-300' : 'bg-gray-100 text-gray-700'}`}>
              BINARY CODE
            </div>
            <textarea
              value={machineValue.binary || 'No binary generated'}
              readOnly
              spellCheck={false}
              className={`flex-1 w-full pl-4 pr-4 text-sm font-mono resize-none focus:outline-none transition-colors ${getEditorBg()} ${getTextColor()} ${getPlaceholderColor()} border-none`}
              style={{ lineHeight: '1.6', tabSize: 2 }}
            />
          </div>

          {/* Hex Section - Narrower since hex is more compact */}
          <div className="w-2/10 flex flex-col">
            <div className={`px-4 py-2 font-semibold text-sm border-b border-gray-600/30 ${isDark ? 'bg-gray-800/50 text-gray-300' : 'bg-gray-100 text-gray-700'}`}>
              HEX CODE
            </div>
            <textarea
              value={machineValue.hex || 'No hex generated'}
              readOnly
              spellCheck={false}
              className={`flex-1 w-full pl-4 pr-4 text-sm font-mono resize-none focus:outline-none transition-colors ${getEditorBg()} ${getTextColor()} ${getPlaceholderColor()} border-none`}
              style={{ lineHeight: '1.6', tabSize: 2 }}
            />
          </div>
        </div>
      );
    }

    // Default textarea for other editors
    return (
      <div className="flex-1 overflow-hidden relative">
        {title.includes('Source') && (
          <div className="absolute top-0 left-0 bottom-0 overflow-y-scroll no-scrollbar pr-2 w-12 text-center text-[14px] leading-[1.6] font-mono overflow-hidden">
            {lines.map((_, index) => (
              <div key={index} className={`${isDark ? 'text-gray-500' : 'text-gray-400'}`}>{index + 1}</div>
            ))}
          </div>
        )}
        <textarea
          value={editorText}
          onChange={(e) => title.includes('Source') && onChange?.(e.target.value) && setEditorText(e.target.value)}
          readOnly={!editable || readOnly || !title.includes('Source')}
          placeholder={placeholder}
          spellCheck={false}
          onScroll={handleScroll}
          className={`w-full h-full pl-9 pr-4 text-sm font-mono resize-none overflow-y-scroll box-content focus:outline-none transition-colors ${getEditorBg()} ${getTextColor()} ${getPlaceholderColor()} border-none shadow-inner rounded-none`}
          style={{ lineHeight: '1.6', tabSize: 2 }}
        />
      </div>
    );
  };

  const displayedLanguage =
    title.includes('Machine') && typeof value !== 'string'
      ? 'Assembly/Binary/Hex'
      : language;

  // Calculate line counts for machine code
  const getMachineCodeLineCounts = () => {
    if (typeof value !== 'string' && title.includes('Machine')) {
      const machineValue = value as MachineCodeValue;
      return {
        assembly: machineValue.assembly?.split('\n').length || 0,
        binary: machineValue.binary?.split('\n').length || 0,
        hex: machineValue.hex?.split('\n').length || 0
      };
    }
    return { assembly: 0, binary: 0, hex: 0 };
  };

  const lineCounts = getMachineCodeLineCounts();

  return (
    <div className={`flex flex-col rounded-xl border ${getBorderColor()} backdrop-blur-sm overflow-hidden transition-colors shadow-md hover:shadow-lg ${get_bg_color()}`}>
      {/* Header */}
      <div className={`flex items-center justify-between px-4 py-3 transition-colors ${isDark ? 'border-gray-800/50 bg-black/30' : 'border-gray-300/70 bg-gray-200'}`}>
        <div className="flex items-center gap-2">
          {getIcon()}
          <div>
            <h3 className="text-sm">{title}</h3>
            <p className={`text-xs transition-colors ${isDark ? 'text-gray-400' : 'text-gray-600'}`}>{subtitle}</p>
          </div>
        </div>
        <div className="flex items-center gap-2">
          <span className={`text-xs transition-colors ${isDark ? 'text-gray-500' : 'text-gray-600'}`}>{displayedLanguage}</span>
          <div className="flex gap-1.5">
            <div className={`w-3 h-3 rounded-full transition-colors ${isDark ? 'bg-gray-600/50' : 'bg-gray-400/50'}`}></div>
            <div className={`w-3 h-3 rounded-full transition-colors ${isDark ? 'bg-gray-600/50' : 'bg-gray-400/50'}`}></div>
            <div className={`w-3 h-3 rounded-full transition-colors ${isDark ? 'bg-gray-600/50' : 'bg-gray-400/50'}`}></div>
          </div>
        </div>
      </div>

      {/* Editor Area */}
      {renderMachineCodeSections()}

      {/* Footer */}
      <div className={`flex items-center justify-between px-4 py-2 transition-colors ${isDark ? 'border-gray-800/50 bg-black/30' : 'border-gray-300/70 bg-gray-200'}`}>
        <span className={`text-xs transition-colors ${isDark ? 'text-gray-500' : 'text-gray-600'}`}>
          {title.includes('Machine') && typeof value !== 'string'
            ? `Assembly: ${lineCounts.assembly} lines | Binary: ${lineCounts.binary} lines | Hex: ${lineCounts.hex} lines`
            : `${lines.length} lines`
          }
        </span>
        <span className={`text-xs transition-colors ${isDark ? 'text-gray-500' : 'text-gray-600'}`}>
          {title.includes('Machine') && typeof value !== 'string'
            ? `${(value.assembly?.length || 0) + (value.binary?.length || 0) + (value.hex?.length || 0)} characters`
            : `${editorText.length} characters`
          }
        </span>
      </div>
    </div>
  );
}