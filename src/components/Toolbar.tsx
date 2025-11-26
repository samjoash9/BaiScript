import { File, FolderOpen, Save, Download, Upload, Zap } from 'lucide-react';


export function Toolbar({
  onImport,
  onExport,
  theme = 'dark',
}: {
  onImport: () => void;
  onExport?: () => void;
  theme?: 'light' | 'dark';
}) {
  const isDark = theme === 'dark';

  return (
    <div className={`border-b backdrop-blur-sm transition-colors ${isDark
      ? 'border-gray-800 bg-black/50'
      : 'border-gray-300/70 bg-gray-200'
      }`}>
      <div className="flex items-center gap-1 px-4 py-2">
        <div className={`w-px h-6 mx-2 transition-colors ${isDark ? 'bg-gray-800' : 'bg-gray-300'}`} />

        <ToolbarButton
          icon={<Upload className="w-4 h-4" />}
          label="Import"
          onClick={onImport}
          theme={theme}
        />

        <ToolbarButton
          icon={<Download className="w-4 h-4" />}
          label="Export"
          onClick={onExport}  // ← pass the handler here
          theme={theme}
        />

        <div className={`w-px h-6 mx-2 transition-colors ${isDark ? 'bg-gray-800' : 'bg-gray-300'}`} />
      </div>
    </div>
  );
}


function ToolbarButton({
  icon,
  label,
  onClick,
  theme = 'dark',
}: {
  icon: React.ReactNode;
  label: string;
  onClick?: () => void;
  theme?: 'light' | 'dark';
}) {
  const isDark = theme === 'dark';

  return (
    <button
      onClick={onClick}
      className={`flex items-center gap-2 px-3 py-1.5 rounded-lg transition-all text-sm ${isDark
        ? 'text-gray-300 hover:text-gray-100 hover:bg-gray-800/50'
        : 'text-gray-700 hover:text-gray-900 hover:bg-gray-200/50'
        }`}
    >
      {icon}
      <span>{label}</span>
    </button>
  );
}

