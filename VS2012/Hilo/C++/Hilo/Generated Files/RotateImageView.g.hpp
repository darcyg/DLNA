﻿

//------------------------------------------------------------------------------
//     This code was generated by a tool.
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
//------------------------------------------------------------------------------
#include "pch.h"
#include "RotateImageView.xaml.h"




void ::Hilo::RotateImageView::InitializeComponent()
{
    if (_contentLoaded)
        return;

    _contentLoaded = true;

    // Call LoadComponent on ms-appx:///RotateImageView.xaml
    ::Windows::UI::Xaml::Application::LoadComponent(this, ref new ::Windows::Foundation::Uri(L"ms-appx:///RotateImageView.xaml"), ::Windows::UI::Xaml::Controls::Primitives::ComponentResourceLocation::Application);

    // Get the Grid named 'ContentRoot'
    ContentRoot = safe_cast<::Windows::UI::Xaml::Controls::Grid^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"ContentRoot"));
    // Get the Grid named 'RotateImageGrid'
    RotateImageGrid = safe_cast<::Windows::UI::Xaml::Controls::Grid^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"RotateImageGrid"));
    // Get the Grid named 'SnappedRotateImageGrid'
    SnappedRotateImageGrid = safe_cast<::Windows::UI::Xaml::Controls::Grid^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"SnappedRotateImageGrid"));
    // Get the TextBlock named 'PageTitle'
    PageTitle = safe_cast<::Windows::UI::Xaml::Controls::TextBlock^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"PageTitle"));
    // Get the Image named 'SnappedPhoto'
    SnappedPhoto = safe_cast<::Windows::UI::Xaml::Controls::Image^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"SnappedPhoto"));
    // Get the RotateTransform named 'SnappedImageRotateTransform'
    SnappedImageRotateTransform = safe_cast<::Windows::UI::Xaml::Media::RotateTransform^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"SnappedImageRotateTransform"));
    // Get the Image named 'Photo'
    Photo = safe_cast<::Windows::UI::Xaml::Controls::Image^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"Photo"));
    // Get the RotateTransform named 'ImageRotateTransform'
    ImageRotateTransform = safe_cast<::Windows::UI::Xaml::Media::RotateTransform^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"ImageRotateTransform"));
    // Get the VisualStateGroup named 'ApplicationViewStates'
    ApplicationViewStates = safe_cast<::Windows::UI::Xaml::VisualStateGroup^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"ApplicationViewStates"));
    // Get the VisualState named 'FullScreenLandscape'
    FullScreenLandscape = safe_cast<::Windows::UI::Xaml::VisualState^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"FullScreenLandscape"));
    // Get the VisualState named 'Filled'
    Filled = safe_cast<::Windows::UI::Xaml::VisualState^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"Filled"));
    // Get the VisualState named 'FullScreenPortrait'
    FullScreenPortrait = safe_cast<::Windows::UI::Xaml::VisualState^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"FullScreenPortrait"));
    // Get the VisualState named 'Snapped'
    Snapped = safe_cast<::Windows::UI::Xaml::VisualState^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"Snapped"));
    // Get the AppBar named 'EditImageBottomAppBar'
    EditImageBottomAppBar = safe_cast<::Windows::UI::Xaml::Controls::AppBar^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"EditImageBottomAppBar"));
    // Get the Button named 'CancelButton'
    CancelButton = safe_cast<::Windows::UI::Xaml::Controls::Button^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"CancelButton"));
    // Get the Button named 'CancelButtonNoLabel'
    CancelButtonNoLabel = safe_cast<::Windows::UI::Xaml::Controls::Button^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"CancelButtonNoLabel"));
    // Get the Button named 'SaveButton'
    SaveButton = safe_cast<::Windows::UI::Xaml::Controls::Button^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"SaveButton"));
    // Get the Button named 'SaveButtonNoLabel'
    SaveButtonNoLabel = safe_cast<::Windows::UI::Xaml::Controls::Button^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"SaveButtonNoLabel"));
    // Get the Button named 'RotateButton'
    RotateButton = safe_cast<::Windows::UI::Xaml::Controls::Button^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"RotateButton"));
    // Get the Button named 'RotateButtonNoLabel'
    RotateButtonNoLabel = safe_cast<::Windows::UI::Xaml::Controls::Button^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"RotateButtonNoLabel"));
    // Get the Button named 'RotateButtonPortrait'
    RotateButtonPortrait = safe_cast<::Windows::UI::Xaml::Controls::Button^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"RotateButtonPortrait"));
}

void ::Hilo::RotateImageView::Connect(int connectionId, Platform::Object^ target)
{
    (void)connectionId; // Unused parameter
    (void)target; // Unused parameter
    _contentLoaded = true;
}

