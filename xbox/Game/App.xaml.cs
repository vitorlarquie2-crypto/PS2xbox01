using Windows.ApplicationModel.Activation;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
namespace PS2Xbox.Game
{
    sealed partial class App : Application
    {
        public App()
        {
            InitializeComponent();
            RequiresPointerMode = ApplicationRequiresPointerMode.WhenRequested;
            Suspending += async (sender, args) =>
            {
                var deferral = args.SuspendingOperation.GetDeferral();
                try { if ((Window.Current.Content as Frame)?.Content is MainPage page) await page.StopAsync(); }
                finally { deferral.Complete(); }
            };
        }
        protected override void OnLaunched(LaunchActivatedEventArgs args)
        {
            if (!(Window.Current.Content is Frame frame))
            {
                frame = new Frame();
                Window.Current.Content = frame;
                SystemNavigationManager.GetForCurrentView().BackRequested += (sender, e) => e.Handled = true;
            }
            if (frame.Content == null) frame.Navigate(typeof(MainPage));
            Window.Current.Activate();
        }
    }
}
