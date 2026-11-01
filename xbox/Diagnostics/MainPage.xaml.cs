using System;
using System.Globalization;
using Windows.Gaming.Input;
using Windows.System.Profile;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
namespace PS2Xbox.Diagnostics
{
    public sealed partial class MainPage : Page
    {
        private readonly DispatcherTimer timer = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(50) };
        private Gamepad active;
        private GamepadButtons previous;
        private int presses;
        private bool alternate;
        public MainPage()
        {
            InitializeComponent();
            Device.Text = "Plataforma: " + AnalyticsInfo.VersionInfo.DeviceFamily + " • pacote x64 0.1.0.0";
            timer.Tick += Poll;
            Loaded += (s, e) => timer.Start();
            Unloaded += (s, e) => { timer.Stop(); active = null; previous = GamepadButtons.None; };
        }
        private void Poll(object sender, object e)
        {
            var pads = Gamepad.Gamepads;
            Count.Text = "Pressões de A: " + presses;
            if (pads.Count == 0)
            {
                active = null; previous = GamepadButtons.None;
                Status.Text = "Nenhum controle detectado. Conecte o controle ao Xbox.";
                Input.Text = "Aguardando entrada...";
                return;
            }
            var pad = pads[0];
            if (active != pad) { active = pad; previous = GamepadButtons.None; }
            var value = pad.GetCurrentReading();
            var down = value.Buttons & ~previous;
            if ((down & GamepadButtons.A) != 0)
            {
                presses++; alternate = !alternate;
                Indicator.Background = new SolidColorBrush(alternate ? Colors.LimeGreen : Colors.SteelBlue);
            }
            if ((down & GamepadButtons.B) != 0) presses = 0;
            previous = value.Buttons;
            Status.Text = "Controle conectado • " + pads.Count + " detectado(s)";
            Input.Text = string.Format(CultureInfo.InvariantCulture,
                "Botões: {0}\nEsquerdo: {1:F2}, {2:F2} • Direito: {3:F2}, {4:F2}\nGatilhos: {5:F2}, {6:F2}",
                value.Buttons, value.LeftThumbstickX, value.LeftThumbstickY,
                value.RightThumbstickX, value.RightThumbstickY, value.LeftTrigger, value.RightTrigger);
            Count.Text = "Pressões de A: " + presses;
        }
    }
}
