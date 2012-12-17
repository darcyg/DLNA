﻿

//------------------------------------------------------------------------------
//     This code was generated by a tool.
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
//------------------------------------------------------------------------------
#include "pch.h"
#include "MainHubView.xaml.h"




void ::Hilo::MainHubView::InitializeComponent()
{
    if (_contentLoaded)
        return;

    _contentLoaded = true;

    // Call LoadComponent on ms-appx:///MainHubView.xaml
    ::Windows::UI::Xaml::Application::LoadComponent(this, ref new ::Windows::Foundation::Uri(L"ms-appx:///MainHubView.xaml"), ::Windows::UI::Xaml::Controls::Primitives::ComponentResourceLocation::Application);

    // Get the CollectionViewSource named 'GroupedItemsViewSource'
    GroupedItemsViewSource = safe_cast<::Windows::UI::Xaml::Data::CollectionViewSource^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"GroupedItemsViewSource"));
    // Get the Grid named 'ContentRoot'
    ContentRoot = safe_cast<::Windows::UI::Xaml::Controls::Grid^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"ContentRoot"));
    // Get the VariableGridView named 'ItemGridView'
    ItemGridView = safe_cast<::Hilo::VariableGridView^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"ItemGridView"));
    // Get the ListView named 'SnapItemListView'
    SnapItemListView = safe_cast<::Windows::UI::Xaml::Controls::ListView^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"SnapItemListView"));
    // Get the Button named 'BackButton'
    BackButton = safe_cast<::Windows::UI::Xaml::Controls::Button^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"BackButton"));
    // Get the TextBlock named 'PageTitle'
    PageTitle = safe_cast<::Windows::UI::Xaml::Controls::TextBlock^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"PageTitle"));
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
    // Get the AppBar named 'MainHubViewBottomAppBar'
    MainHubViewBottomAppBar = safe_cast<::Windows::UI::Xaml::Controls::AppBar^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"MainHubViewBottomAppBar"));
    // Get the Button named 'RotateButton'
    RotateButton = safe_cast<::Windows::UI::Xaml::Controls::Button^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"RotateButton"));
    // Get the Button named 'CropButton'
    CropButton = safe_cast<::Windows::UI::Xaml::Controls::Button^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"CropButton"));
    // Get the Button named 'CartoonizeButton'
    CartoonizeButton = safe_cast<::Windows::UI::Xaml::Controls::Button^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"CartoonizeButton"));
    // Get the Button named 'RotateButtonNoLabel'
    RotateButtonNoLabel = safe_cast<::Windows::UI::Xaml::Controls::Button^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"RotateButtonNoLabel"));
    // Get the Button named 'CropButtonNoLabel'
    CropButtonNoLabel = safe_cast<::Windows::UI::Xaml::Controls::Button^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"CropButtonNoLabel"));
    // Get the Button named 'CartoonizeButtonNoLabel'
    CartoonizeButtonNoLabel = safe_cast<::Windows::UI::Xaml::Controls::Button^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"CartoonizeButtonNoLabel"));
}

void ::Hilo::MainHubView::Connect(int connectionId, Platform::Object^ target)
{
    switch (connectionId)
    {
    case 1:
        (safe_cast<::Windows::UI::Xaml::Controls::ListViewBase^>(target))->ItemClick +=
            ref new ::Windows::UI::Xaml::Controls::ItemClickEventHandler(this, (void (::Hilo::MainHubView::*)(Platform::Object^, Windows::UI::Xaml::Controls::ItemClickEventArgs^))&MainHubView::OnPhotoItemClicked);
        break;
    case 2:
        (safe_cast<::Windows::UI::Xaml::Controls::ListViewBase^>(target))->ItemClick +=
            ref new ::Windows::UI::Xaml::Controls::ItemClickEventHandler(this, (void (::Hilo::MainHubView::*)(Platform::Object^, Windows::UI::Xaml::Controls::ItemClickEventArgs^))&MainHubView::OnPhotoItemClicked);
        break;
    case 3:
        (safe_cast<::Windows::UI::Xaml::Controls::Primitives::ButtonBase^>(target))->Click +=
            ref new ::Windows::UI::Xaml::RoutedEventHandler(this, (void (::Hilo::MainHubView::*)(Platform::Object^, Windows::UI::Xaml::RoutedEventArgs^))&MainHubView::GoBack);
        break;
    }
    (void)connectionId; // Unused parameter
    (void)target; // Unused parameter
    _contentLoaded = true;
}
