using System;
using System.IO;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.Gaming.Input;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media.Imaging;
namespace PS2Xbox.Game
{
    public sealed partial class MainPage : Page
    {
        readonly SemaphoreSlim lifecycle = new SemaphoreSlim(1, 1);
        readonly DispatcherTimer timer = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(16) };
        readonly byte[] pixels = new byte[640 * 512 * 4], error = new byte[1024];
        WriteableBitmap bitmap;
        ulong lastFrame;
        bool running, active = true;
        public MainPage()
        {
            InitializeComponent();
            timer.Tick += Tick;
            Window.Current.Activated += (sender, args) =>
            {
                active = args.WindowActivationState != CoreWindowActivationState.Deactivated;
                if (!active) Native.ps2x_set_input(0, 0, 0, 0, 0);
            };
        }
        void Busy(bool busy) { ImportButton.IsEnabled = StartButton.IsEnabled = !busy; }
        async void Import_Click(object sender, RoutedEventArgs e)
        {
            if (!await lifecycle.WaitAsync(0)) return;
            Busy(true);
            try { Status.Text = "Importando arquivos do jogo…"; await GameFiles.ImportAsync(); Status.Text = "Importação concluída. Selecione Iniciar jogo."; }
            catch (Exception ex) { Status.Text = ex.Message; }
            finally { Busy(false); lifecycle.Release(); }
        }
        async void Start_Click(object sender, RoutedEventArgs e)
        {
            if (!await lifecycle.WaitAsync(0)) return;
            Busy(true);
            try
            {
                var elf = await GameFiles.FindAsync();
                if (Native.ps2x_start(Encoding.UTF8.GetBytes(elf + "\0")) == 0) throw new IOException("Não foi possível iniciar o runtime.");
                lastFrame = 0; running = true;
                Panel.Visibility = Visibility.Collapsed;
                timer.Start();
            }
            catch (Exception ex) { Status.Text = ex.Message; }
            finally { Busy(false); lifecycle.Release(); }
        }
        public async Task StopAsync()
        {
            await lifecycle.WaitAsync();
            try
            {
                timer.Stop(); running = false;
                Native.ps2x_set_input(0, 0, 0, 0, 0);
                await Task.Run(() => Native.ps2x_stop());
                Panel.Visibility = Visibility.Visible;
            }
            finally { lifecycle.Release(); }
        }
        async void Tick(object sender, object args)
        {
            if (!running) return;
            try
            {
                var state = Native.ps2x_status(error, (uint)error.Length);
                if (state != 1)
                {
                    var end = Array.IndexOf(error, (byte)0);
                    var message = state < 0 ? Encoding.UTF8.GetString(error, 0, end < 0 ? error.Length : end) : "A execução foi encerrada.";
                    await StopAsync(); Status.Text = message; return;
                }
                ReadInput();
                var sequence = Native.ps2x_copy_frame(pixels, (uint)pixels.Length, out uint width, out uint height);
                if (sequence == 0 || sequence == lastFrame || width == 0 || height == 0 || width > 640 || height > 512) return;
                if (bitmap == null || bitmap.PixelWidth != width || bitmap.PixelHeight != height)
                {
                    bitmap = new WriteableBitmap((int)width, (int)height); Screen.Source = bitmap;
                }
                using (var stream = bitmap.PixelBuffer.AsStream()) stream.Write(pixels, 0, checked((int)(width * height * 4)));
                bitmap.Invalidate(); lastFrame = sequence;
            }
            catch (Exception ex) { await StopAsync(); Status.Text = ex.Message; }
        }
        void ReadInput()
        {
            if (!active || Gamepad.Gamepads.Count == 0) { Native.ps2x_set_input(0, 0, 0, 0, 0); return; }
            var pad = Gamepad.Gamepads[0].GetCurrentReading();
            uint buttons = 0;
            Action<GamepadButtons, int> map = (button, bit) => { if ((pad.Buttons & button) != 0) buttons |= 1u << bit; };
            map(GamepadButtons.DPadUp, 1); map(GamepadButtons.DPadRight, 2);
            map(GamepadButtons.DPadDown, 3); map(GamepadButtons.DPadLeft, 4);
            map(GamepadButtons.Y, 5); map(GamepadButtons.B, 6); map(GamepadButtons.A, 7); map(GamepadButtons.X, 8);
            map(GamepadButtons.LeftShoulder, 9); map(GamepadButtons.RightShoulder, 11);
            if (pad.LeftTrigger > 0.5) buttons |= 1u << 10;
            if (pad.RightTrigger > 0.5) buttons |= 1u << 12;
            map(GamepadButtons.View, 13); map(GamepadButtons.Menu, 15);
            map(GamepadButtons.LeftThumbstick, 16); map(GamepadButtons.RightThumbstick, 17);
            Native.ps2x_set_input(buttons, (float)pad.LeftThumbstickX, -(float)pad.LeftThumbstickY,
                (float)pad.RightThumbstickX, -(float)pad.RightThumbstickY);
        }
    }
}
