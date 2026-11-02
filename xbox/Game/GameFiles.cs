using System;
using System.IO;
using System.IO.Compression;
using System.Security.Cryptography;
using System.Threading.Tasks;
using Windows.Storage;
namespace PS2Xbox.Game
{
    internal static class GameFiles
    {
        const string ElfName = "SCUS_974.72";
        const string ElfHash = "d16ad76ba8cdf87695d94b1316d51c1f248959ec0749814c31bb7a2d55d9a2e1";
        static string Root => Path.Combine(ApplicationData.Current.LocalFolder.Path, "Game");
        internal static async Task<string> FindAsync()
        {
            return await Task.Run(() =>
            {
                return Validate(Root);
            });
        }
        static string Validate(string directory)
        {
                if (!Directory.Exists(directory)) throw new IOException("Importe Game.zip antes de iniciar.");
                var files = Directory.GetFiles(directory, ElfName, SearchOption.AllDirectories);
                if (files.Length != 1) throw new IOException("O conteúdo deve incluir exatamente um SCUS_974.72.");
                using (var stream = File.OpenRead(files[0]))
                using (var sha = SHA256.Create())
                    if (BitConverter.ToString(sha.ComputeHash(stream)).Replace("-", "").ToLowerInvariant() != ElfHash)
                        throw new IOException("O ELF não corresponde à versão recompilada neste pacote.");
                var disc = Path.GetDirectoryName(files[0]);
                foreach (var required in new[] { "NICO.DAT", "GAMECORE.XFF", "KERNEL.XFF", "MANAGER.XFF", "STARTUP.XFF", "IOPRP300.IMG" })
                    if (!File.Exists(Path.Combine(disc, required)))
                        throw new IOException("Arquivo obrigatório ausente: " + required);
                return files[0];
        }
        internal static async Task ImportAsync()
        {
            await Task.Run(() =>
            {
                if (Directory.Exists(Root)) throw new IOException("A pasta Game já existe. Os arquivos e saves foram preservados.");
                var archivePath = Path.Combine(ApplicationData.Current.LocalFolder.Path, "Game.zip");
                if (!File.Exists(archivePath)) throw new IOException("Envie Game.zip para LocalState pelo Xbox Device Portal.");
                var staging = Root + ".importing";
                if (Directory.Exists(staging)) Directory.Delete(staging, true);
                Directory.CreateDirectory(staging);
                try
                {
                    var prefix = Path.GetFullPath(staging) + Path.DirectorySeparatorChar;
                    using (var archive = ZipFile.OpenRead(archivePath))
                    {
                        if (archive.Entries.Count > 4096) throw new IOException("O arquivo contém entradas demais.");
                        long total = 0;
                        foreach (var entry in archive.Entries)
                        {
                            total = checked(total + entry.Length);
                            if (total > 12L * 1024 * 1024 * 1024) throw new IOException("O conteúdo excede 12 GiB.");
                            var name = entry.FullName.Replace('/', Path.DirectorySeparatorChar);
                            if (name.Contains(":") || Path.IsPathRooted(name)) throw new IOException("Caminho inválido no ZIP.");
                            var destination = Path.GetFullPath(Path.Combine(staging, name));
                            if (!destination.StartsWith(prefix, StringComparison.OrdinalIgnoreCase)) throw new IOException("Caminho fora da pasta Game.");
                            if (string.IsNullOrEmpty(entry.Name)) Directory.CreateDirectory(destination);
                            else
                            {
                                Directory.CreateDirectory(Path.GetDirectoryName(destination));
                                using (var input = entry.Open())
                                using (var output = new FileStream(destination, FileMode.CreateNew, FileAccess.Write)) input.CopyTo(output);
                            }
                        }
                    }
                    Validate(staging);
                    Directory.Move(staging, Root);
                }
                finally { if (Directory.Exists(staging)) Directory.Delete(staging, true); }
            });
        }
    }
}
