using System.Runtime.InteropServices;
namespace PS2Xbox.Game
{
    internal static class Native
    {
        const string Library = "PS2XboxRuntime.dll";
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ps2x_start(byte[] elfPath);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ps2x_stop();
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ps2x_status(byte[] error, uint capacity);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern ulong ps2x_copy_frame([Out] byte[] pixels, uint capacity, out uint width, out uint height);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ps2x_set_input(uint buttons, float lx, float ly, float rx, float ry);
    }
}
